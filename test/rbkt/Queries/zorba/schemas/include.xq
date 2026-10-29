import schema namespace s = 
    "http://test.example.com/project1/schemas/simple" at "incl-extensions.xsd";

validate { <s:superproduct string="blabla" something="something"/> }
