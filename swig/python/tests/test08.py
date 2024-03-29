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

import sys
sys.path.insert(0, '@pythonPath@')
import zorba_api

 
def test(zorba): 
  #Read and write result 
  print 'Executing: test08.xq' 
  lines = '(1, xs:int(2),"bla", <a><b att="{(3, xs:int(4),"foo", "bar")}"/>text<!--a comment--><?pi?></a>)' 
  xquery = zorba.compileQuery(lines) 
  result = xquery.execute() 
  print result 
 
  print "Iterator:" 
  iterator = xquery.iterator() 

  printIterator(iterator, "")
 
  f = open('test8_result.xml', 'w') 
  f.write(result) 
  f.close() 
  return 


def printIterator(iterator, pre):
  item = zorba_api.Item() 
  iterator.open() 
  while iterator.next(item):
    printItem(item, pre)
  iterator.close()

 
def printItem(item, pre):
  if item.isAtomic():
    typeItem = item.getType()
    print pre + "Leaf Atomic: '" + item.getStringValue() + "' \ttype:", typeItem.getStringValue()   
    return

  if item.isNode():
    kind = item.getNodeKind()
  
    if kind ==  0: # anyNode
      print pre + 'Any node'
      print pre + "  Children:"
      printIterator(item.getChildren(), pre+"    ")

    if kind ==  1: # doc
      print pre + 'Doc'
      print pre + "  Children:"
      printIterator(item.getChildren(), pre+"    ")

    if kind == 2: # element
      nodeName = zorba_api.Item()
      item.getNodeName(nodeName)
      typeItem = item.getType()              
      print pre + "Start Element: ", nodeName.getStringValue(), " \ttype:", typeItem.getStringValue()
      print pre + "  Attributes:"
      printIterator(item.getAttributes(), pre+"    ")
      print pre + "  Children:"
      printIterator(item.getChildren(), pre+"    ")
      print pre + "End  Element: ", nodeName.getStringValue()        
    
    if kind == 3: # attribute
      nodeName = zorba_api.Item()
      item.getNodeName(nodeName)
      typeItem = item.getType()              
      print pre + "Attribute: ", nodeName.getStringValue(), "= '" + item.getStringValue() + "'"" \ttype:", typeItem.getStringValue()
      print pre+"  Atomization value:"
      printIterator(item.getAtomizationValue(), pre+"    ")
    
    if kind == 4: # text
      typeItem = item.getType()              
      print pre + "Text: ", item.getStringValue(), " \ttype:", typeItem.getStringValue()
      print pre+"  Atomization value:"
      printIterator(item.getAtomizationValue(), pre+"    ")

    if kind == 5: # pi
      nodeName = zorba_api.Item()
      item.getNodeName(nodeName)
      print pre + "Pi: ", nodeName.getStringValue()
    
    if kind == 6: # comment
      print pre + "Comment: ", item.getStringValue()
  else:
    print pre+"Item not Node, not Atomic"


 
store = zorba_api.InMemoryStore_getInstance() 
zorba = zorba_api.Zorba_getInstance(store) 
 
print "Running: XQuery execute - Get Iterator and print info from its items" 
test(zorba) 
print "Success" 
 
 
zorba.shutdown() 
zorba_api.InMemoryStore_shutdown(store) 