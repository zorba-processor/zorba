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

(: Fetches $url with fn:doc (Network) and prints the error code: the tests
 : use addresses where nothing listens, so an allowed access fails with
 : another error than zerr:ZXQP0029. :)
declare namespace err = "http://www.w3.org/2005/xqt-errors";

declare variable $url external;

try {
  doc($url)
} catch * {
  concat("error:", local-name-from-QName($err:code))
}
