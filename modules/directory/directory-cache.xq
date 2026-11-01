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
 : The document cache hook of the directory module
 : (<code>http://zorba.io/modules/directory</code>).
 : <p/>
 : The directory module asks <code>dc:get</code> for the record of a
 : document before it reads and parses the file, and passes every record it
 : builds to <code>dc:put</code>. This implementation caches nothing: every
 : call reads the file again. An application that keeps a Zorba instance
 : across queries (e.g. <code>zorba-mcp</code>) can resolve this module to
 : external functions of its own that keep the records, validated against
 : the file's size and modification time.
 :
 : @author Zorba contributors
 : @project Zorba/Data Converters/Directory
 :)
module namespace dc = "http://zorba.io/modules/directory-cache";

declare namespace an = "http://zorba.io/annotations";
declare namespace ver = "http://zorba.io/options/versioning";
declare option ver:module-version "1.0";

(:~
 : Returns the cached record of a document, if any.
 :
 : @param $root the root directory the document is opened under.
 : @param $path the path of the document relative to <code>$root</code>.
 : @return the empty sequence (nothing is cached).
 :)
declare %an:nondeterministic function dc:get(
  $root as xs:string,
  $path as xs:string ) as object()?
{
  ()
};

(:~
 : Offers the record of a document to the cache.
 :
 : @param $root the root directory the document was opened under.
 : @param $path the path of the document relative to <code>$root</code>.
 : @param $record the document record.
 : @return <code>$record</code>.
 :)
declare %an:nondeterministic function dc:put(
  $root as xs:string,
  $path as xs:string,
  $record as object() ) as object()
{
  $record
};
