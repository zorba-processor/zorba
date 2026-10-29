import schema namespace d = "http://test.example.com/schema.xsd" at
"schema.xsd";

declare function local:foo($id as d:ContentID) 
{
  $id
};

local:foo(d:ContentID("bar"))
