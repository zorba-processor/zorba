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

(: Opens two XML files with external entities: docs/note.xml refers to an
 : entity file next to it, escape.xml to one outside the root. Prints one
 : string per file: the content, or the error code and message. :)
import module namespace dir = "http://zorba.io/modules/directory";

declare variable $root as xs:string external := $dir:root;

for $path in ("docs/note.xml", "escape.xml")
let $d := dir:open($root, $path)
let $e := $d("error")
return concat($path, "=",
  if (exists($e)) then concat($e("code"), ":", $e("message"))
  else string($d("content")))
