# Copyright 2006-2016 zorba.io
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
# http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# The directory module reads through the EXPath file module.
IF (ZORBA_WITH_FILE_ACCESS)
  SET (_dir_xq "${CMAKE_CURRENT_SOURCE_DIR}/directory.xq")
  IF (ZORBA_NO_XMLSCHEMA)
    # HTML is parsed with x:parse-html, and the xml module is not built without
    # XML Schema support (it imports a schema). Build the module without that
    # import, with HTML files reported as an unsupported format.
    SET_PROPERTY (DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_dir_xq}")
    FILE (READ "${_dir_xq}" _dir_text)
    STRING (REGEX REPLACE
      "\\(: BEGIN-HTML-IMPORT :\\).*\\(: END-HTML-IMPORT :\\)" ""
      _dir_text "${_dir_text}")
    STRING (REGEX REPLACE
      "\\(: BEGIN-HTML-BRANCH :\\).*\\(: END-HTML-BRANCH :\\)"
      "error( xs:QName( \"dir:UNSUPPORTED_FORMAT\" ), \"HTML documents need a build with XML Schema support\" )"
      _dir_text "${_dir_text}")
    SET (_dir_xq "${CMAKE_CURRENT_BINARY_DIR}/directory.xq")
    FILE (WRITE "${_dir_xq}" "${_dir_text}")
  ENDIF (ZORBA_NO_XMLSCHEMA)
  DECLARE_ZORBA_MODULE (FILE "${_dir_xq}" VERSION 1.0
    URI "http://zorba.io/modules/directory")
ENDIF (ZORBA_WITH_FILE_ACCESS)

# vim:set et sw=2 ts=2:
