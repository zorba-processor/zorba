/*
 * Copyright 2006-2016 zorba.io
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ZORBA_RUNTIME_PARSING_AND_SERIALIZING_FRAGMENT_ISTREAM_H
#define ZORBA_RUNTIME_PARSING_AND_SERIALIZING_FRAGMENT_ISTREAM_H

#include <deque>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <libxml/parser.h>
#include <store/api/iterator.h>

namespace zorba {

/**
 * A class to hold an input stream for the parse-xml:parse() function
 * Author: Nicolae Brinza
 */
/**
 * A SAX event recorded by the fragment parser, to be replayed into a loader.
 * Strings are owned copies; libxml2's buffers do not outlive the callback.
 */
struct FragmentEvent
{
  enum kind_type {
    START_ELEMENT,
    END_ELEMENT,
    CHARACTERS,
    CDATA,
    COMMENT,
    PI
  };

  struct attribute
  {
    std::string localname, prefix, uri, value;
    bool has_prefix, has_uri;
  };

  kind_type kind;
  std::string name;                     // localname, or PI target
  std::string prefix, uri;
  bool has_prefix, has_uri;
  std::string text;                     // characters, comment, or PI data
  std::vector<std::pair<std::string,std::string> > namespaces;
  std::vector<bool> ns_has_prefix;
  std::vector<attribute> attributes;
  int nb_defaulted;
};

class FragmentIStream : public std::istream
{
public:
  static const unsigned int DEFAULT_BUFFER_SIZE = 4096;
  static const unsigned int PARSED_NODES_BATCH_SIZE = 1024;
  
  // names of these states are orientative
  enum FRAGMENT_PARSER_STATE {
    FRAGMENT_FIRST_START_DOC = 0,
    FRAGMENT_PROLOG,
    FRAGMENT_CONTENT                   // this state is set once an element is encountered
  };

public:
  std::istringstream* theIss;
  std::istream* theStream;
  StreamReleaser theStreamReleaser;
  std::vector<char> theBuffer;
  unsigned long bytes_in_buffer;
  unsigned long current_offset;
  int current_element_depth;
  int root_elements_to_skip;
  xmlParserCtxtPtr ctxt;
  FRAGMENT_PARSER_STATE state;
  bool forced_parser_stop;
  bool reached_eof;
  unsigned int parsed_nodes_count;
  store::Iterator_t children;
  // Events parsed but not yet replayed into a loader.
  std::deque<FragmentEvent> events;
  bool parser_started;
  bool parser_done;
  int parse_depth;                  // element depth as seen by libxml2, wrapper included
  void* current_loader;             // the FragmentXmlLoader replaying events
  std::vector<std::vector<std::pair<std::string,std::string> > > ns_scopes;

  bool only_one_doc_node;           // If set to true, all parsed fragments will be added to one
                                    // single document node, instead of having one for each fragment.
  
public:
  FragmentIStream()
    :
    std::istream(NULL),
    theIss(NULL),
    theStream(NULL),
    theStreamReleaser(nullptr),
    bytes_in_buffer(0),
    current_offset(0),
    current_element_depth(0),
    root_elements_to_skip(0),
    ctxt(NULL),
    state(FRAGMENT_FIRST_START_DOC),
    forced_parser_stop(false),
    reached_eof(false),
    parsed_nodes_count(0),
    children(NULL),
    parser_started(false),
    parser_done(false),
    parse_depth(0),
    current_loader(NULL),
    only_one_doc_node(false)
  {
  };

  bool stream_is_consumed()
  {
    return parser_done && events.empty();
  }
  
  StreamReleaser getStreamReleaser()
  {
    return theStreamReleaser;
  }
  
  void setStreamReleaser(StreamReleaser aReleaser)
  {
    theStreamReleaser = aReleaser;
  }

  void reset()
  {
    theBuffer.clear();

    if (theIss)
    {
      delete theIss;
    }
    
    if (theStreamReleaser)
    {
      theStreamReleaser(theStream);
    }

    if (ctxt)
    {
      if (ctxt->myDoc)
        xmlFreeDoc(ctxt->myDoc);
      ctxt->myDoc = NULL;
      xmlFreeParserCtxt(ctxt);
    }

    theIss = NULL;
    theStream = NULL;
    bytes_in_buffer = 0;
    current_offset = 0;
    current_element_depth = 0;
    root_elements_to_skip = 0;
    ctxt = NULL;
    state = FRAGMENT_FIRST_START_DOC;
    forced_parser_stop = false;
    reached_eof = false;
    parsed_nodes_count = 0;
    children = NULL;
    events.clear();
    parser_started = false;
    parser_done = false;
    parse_depth = 0;
    current_loader = NULL;
    ns_scopes.clear();
    only_one_doc_node = false;
  }
  
  virtual ~FragmentIStream()
  {
    reset();
  }
};

}
#endif
/*
 * Local variables:
 * mode: c++
 * End:
 */
