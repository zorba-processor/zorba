xquery version "3.0";

(:
 : Copyright 2006-2016 zorba.io
 :
 : Licensed under the Apache License, Version 2.0 (the "License");
 : you may not use this file except in compliance with the License.
 : You may obtain a copy of the License at
 :
 : http://www.apache.org/licenses/LICENSE-2.0
 :
 : Unless required by applicable law or agreed to in writing, software
 : distributed under the License is distributed on an "AS IS" BASIS,
 : WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 : See the License for the specific language governing permissions and
 : limitations under the License.
 :)

(:~
 : Read-only access to a directory tree of XML, JSON and CSV documents.
 : <p/>
 : The documents stay ordinary files. Every call walks the tree again and
 : parses only the documents it returns, so the result always reflects the
 : files as they are on disk; nothing is indexed. An application can keep
 : parsed documents across queries through the hook module
 : <code>http://zorba.io/modules/directory-cache</code>, which serves a
 : document only while its file is unchanged; by default nothing is cached.
 : <p/>
 : Paths are relative to a root directory and use <code>/</code> as the
 : separator. A path must stay inside the root: absolute paths,
 : <code>..</code> segments and paths through a symbolic link are rejected,
 : and the listing does not follow symbolic links. Entries whose name starts
 : with <code>.</code> (e.g. <code>.git</code>) are not listed.
 : <p/>
 : A document is returned as a record:
 : <pre>{ "path" : "orders/2026/q1.xml", "format" : "xml", "content" : ... }</pre>
 : where the content is a document node for XML, the parsed item for JSON
 : (an array of the items if the file holds several top-level items, as in
 : JSON Lines) and an array of row objects for CSV (the first line holds the
 : column names, see <code>csv:parse</code>). If the file cannot be read or
 : parsed, the record carries the error instead of the content, so that one
 : malformed file does not stop a query over the whole tree:
 : <pre>{ "path" : "broken/bad.json", "format" : "json",
 :   "error" : { "code" : "...", "namespace" : "...", "message" : "..." } }</pre>
 : The root can be passed to every function or bound once to the external
 : variable <code>$dir:root</code> (the <code>zorba --directory</code>
 : command-line option does so).
 : <p/>
 : The module reads files with the EXPath file module, so the security
 : policy that the query runs under applies to every access: with a read
 : path allow-list, files outside it cannot be read even through a symbolic
 : link.
 :
 : @author Zorba contributors
 : @project Zorba/Data Converters/Directory
 :)
module namespace dir = "http://zorba.io/modules/directory";

import module namespace file = "http://expath.org/ns/file";
import module namespace jn = "http://jsoniq.org/functions";
import module namespace csv = "http://zorba.io/modules/json-csv";
import module namespace dc = "http://zorba.io/modules/directory-cache";

declare namespace an = "http://zorba.io/annotations";
declare namespace err = "http://www.w3.org/2005/xqt-errors";
declare namespace ver = "http://zorba.io/options/versioning";
declare option ver:module-version "1.0";

(:~
 : The root directory, for queries that do not pass one explicitly, e.g.
 : <code>dir:list($dir:root)</code>. Bound by <code>zorba --directory</code>
 : or by the embedding application; the empty string (the default) means
 : that no root is bound.
 :)
declare variable $dir:root as xs:string external := "";

(:===========================================================================:)

(:~
 : Lists the regular files under a root directory, recursively.
 :
 : @param $root the root directory.
 : @return one object per file, sorted by path:
 : <code>{ "path" : string, "format" : string or null, "size" : integer,
 : "modified" : string (xs:dateTime) }</code>. The format is null for an
 : extension that the module does not parse (see <code>dir:format</code>).
 : Hidden entries and symbolic links are skipped.
 : @error dir:NO_ROOT if <code>$root</code> is the empty string.
 : @error dir:NOT_FOUND if <code>$root</code> is not a directory.
 :)
declare %an:nondeterministic function dir:list( $root as xs:string )
  as object()*
{
  let $r := dir:check-root( $root )
  for $path in dir:walk( $r, "" )
  let $full := concat( $r, "/", $path )
  order by $path
  return {
    "path" : $path,
    "format" : dir:format-or-null( $path ),
    "size" : file:size( $full ),
    "modified" : string( file:last-modified( $full ) )
  }
};

(:~
 : Opens one document.
 :
 : @param $root the root directory.
 : @param $path the path of the document relative to <code>$root</code>;
 : empty and <code>.</code> segments are ignored.
 : @return the document record (see the module description). A file that
 : cannot be parsed, or whose format is not supported, yields a record with
 : an <code>error</code> member instead of <code>content</code>.
 : @error dir:NO_ROOT if <code>$root</code> is the empty string.
 : @error dir:NOT_FOUND if <code>$root</code> is not a directory or
 : <code>$path</code> is not a regular file.
 : @error dir:INVALID_PATH if <code>$path</code> is empty, absolute, contains
 : a <code>..</code> segment or goes through a symbolic link.
 :)
declare %an:nondeterministic function dir:open(
  $root as xs:string,
  $path as xs:string ) as object()
{
  let $r := dir:check-root( $root )
  return dir:load( $r, dir:resolve( $r, $path ) )
};

(:~
 : Returns the records of all documents with a known format under a root
 : directory, sorted by path.
 :
 : @param $root the root directory.
 : @return the document records (see <code>dir:open</code>).
 : @error dir:NO_ROOT if <code>$root</code> is the empty string.
 : @error dir:NOT_FOUND if <code>$root</code> is not a directory.
 :)
declare %an:nondeterministic function dir:documents( $root as xs:string )
  as object()*
{
  dir:documents( $root, ( "xml", "json", "csv", "html" ) )
};

(:~
 : Returns the records of the documents of the given formats under a root
 : directory, sorted by path. Each record carries the path of its file, so
 : a FLWOR expression can join documents of different files and formats.
 :
 : @param $root the root directory.
 : @param $formats the formats to return (see <code>dir:format</code>).
 : @return the document records (see <code>dir:open</code>).
 : @error dir:NO_ROOT if <code>$root</code> is the empty string.
 : @error dir:NOT_FOUND if <code>$root</code> is not a directory.
 :)
declare %an:nondeterministic function dir:documents(
  $root as xs:string,
  $formats as xs:string* ) as object()*
{
  let $r := dir:check-root( $root )
  (: Sort the paths before loading: an order by over the loaded records
   : would hold every parsed document until the last one is parsed. :)
  let $paths :=
    for $path in dir:walk( $r, "" )
    where dir:format( $path ) = $formats
    order by $path
    return $path
  for $path in $paths
  return dir:load( $r, $path )
};

(:~
 : Returns the format of a path, by extension (case-insensitive):
 : <code>xml</code> for .xml and .xhtml, <code>json</code> for .json,
 : .jsonl and .ndjson, <code>csv</code> for .csv, <code>html</code> for
 : .html and .htm.
 :
 : @param $path a path.
 : @return the format, or the empty sequence for any other extension.
 :)
declare function dir:format( $path as xs:string ) as xs:string?
{
  let $name := tokenize( $path, "/" )[last()]
  let $ext :=
    if ( contains( $name, "." ) )
    then lower-case( replace( $name, "^.*\.", "" ) )
    else ""
  return
    switch ( $ext )
    case "xml" case "xhtml" return "xml"
    case "json" case "jsonl" case "ndjson" return "json"
    case "csv" return "csv"
    case "html" case "htm" return "html"
    default return ()
};

(:===========================================================================:)

(:~
 : The root without trailing separators, after checking that it is a
 : directory.
 :)
declare %private %an:nondeterministic function dir:check-root(
  $root as xs:string ) as xs:string
{
  if ( $root eq "" ) then
    error( xs:QName( "dir:NO_ROOT" ),
      "no root directory: pass one or bind $dir:root" )
  else
    let $r :=
      if ( matches( $root, "[^/]/+$" ) )
      then replace( $root, "/+$", "" )
      else $root
    return
      if ( file:is-directory( $r ) ) then $r
      else error( xs:QName( "dir:NOT_FOUND" ),
        concat( "root is not a directory: ", $root ) )
};

(:~
 : The relative paths of the regular files under $root/$rel. Hidden entries
 : and symbolic links are skipped; a directory that cannot be listed is
 : skipped too.
 :)
declare %private %an:nondeterministic function dir:walk(
  $root as xs:string,
  $rel as xs:string ) as xs:string*
{
  let $base := if ( $rel eq "" ) then $root else concat( $root, "/", $rel )
  for $name in dir:children( $base )
  where not( starts-with( $name, "." ) )
  let $path := if ( $rel eq "" ) then $name else concat( $rel, "/", $name )
  let $full := concat( $root, "/", $path )
  return
    if ( dir:is-link( $full ) ) then ()
    else if ( file:is-directory( $full ) ) then dir:walk( $root, $path )
    else if ( file:is-file( $full ) ) then $path
    else ()
};

declare %private %an:nondeterministic function dir:children(
  $dir as xs:string ) as xs:string*
{
  try { file:list( $dir ) } catch * { () }
};

(:~
 : Whether a path is a symbolic link. A path that the security policy does
 : not let us inspect (e.g. a link whose target is outside the read paths)
 : counts as a link: it is not followed either way.
 :)
declare %private %an:nondeterministic function dir:is-link(
  $full as xs:string ) as xs:boolean
{
  try { file:is-symlink( $full ) } catch * { true() }
};

declare %private function dir:invalid-path(
  $path as xs:string,
  $why as xs:string )
{
  error( xs:QName( "dir:INVALID_PATH" ),
    concat( "path outside the root (", $why, "): ", $path ) )
};

(:~
 : Normalizes a relative path and checks that it stays inside $root and
 : names a regular file.
 :)
declare %private %an:nondeterministic function dir:resolve(
  $root as xs:string,
  $path as xs:string ) as xs:string
{
  if ( starts-with( $path, "/" ) ) then
    dir:invalid-path( $path, "absolute path" )
  else
    let $segments := tokenize( $path, "/" )[ . ne "" and . ne "." ]
    let $rel := string-join( $segments, "/" )
    return
      if ( empty( $segments ) ) then
        dir:invalid-path( $path, "empty path" )
      else if ( $segments = ".." ) then
        dir:invalid-path( $path, "'..' segment" )
      else if ( some $i in 1 to count( $segments ) satisfies
                dir:is-link( concat( $root, "/",
                  string-join( subsequence( $segments, 1, $i ), "/" ) ) ) ) then
        dir:invalid-path( $path, "symbolic link" )
      else if ( not( file:is-file( concat( $root, "/", $rel ) ) ) ) then
        error( xs:QName( "dir:NOT_FOUND" ), concat( "no such file: ", $path ) )
      else
        $rel
};

declare %private function dir:format-or-null( $path as xs:string ) as item()
{
  ( dir:format( $path ), jn:null() )[1]
};

(:~
 : The record of the document at $root/$path (already resolved), from the
 : document cache hook if it has one.
 :)
declare %private %an:nondeterministic function dir:load(
  $root as xs:string,
  $path as xs:string ) as object()
{
  let $cached := dc:get( $root, $path )
  return
    if ( exists( $cached ) ) then $cached
    else dc:put( $root, $path, dir:read( $root, $path ) )
};

declare %private %an:nondeterministic function dir:read(
  $root as xs:string,
  $path as xs:string ) as object()
{
  let $format := dir:format( $path )
  return
    try {
      {
        "path" : $path,
        "format" : dir:format-or-null( $path ),
        "content" : dir:parse( concat( $root, "/", $path ), $format )
      }
    } catch * {
      {
        "path" : $path,
        "format" : dir:format-or-null( $path ),
        "error" : {
          "code" : local-name-from-QName( $err:code ),
          "namespace" : namespace-uri-from-QName( $err:code ),
          "message" : string( $err:description )
        }
      }
    }
};

declare %private %an:nondeterministic function dir:parse(
  $full as xs:string,
  $format as xs:string? ) as item()
{
  switch ( $format )
  case "xml" return
    parse-xml( file:read-text( $full ) )
  case "json" return
    let $items := jn:parse-json( file:read-text( $full ) )
    return if ( count( $items ) eq 1 ) then $items else [ $items ]
  case "csv" return
    [ csv:parse( file:read-text( $full ) ) ]
  case "html" return
    (: HOOK(parse-html): parse with parse-html() once it is available, e.g.
     : parse-html( file:read-text( $full ) ). Until then HTML files are
     : listed and reported as unsupported. :)
    error( xs:QName( "dir:UNSUPPORTED_FORMAT" ),
      "HTML documents are not parsed yet (parse-html() is not available)" )
  default return
    error( xs:QName( "dir:UNSUPPORTED_FORMAT" ),
      "no parser for this file extension" )
};
