{
  "$invalid" : true, 
  "$expected" : "Q{http://www.example.com/my-schema}foo", 
  "$value" : {
    "k1" : 41
  }, 
  "$reason" : "value violates $constraints facet \"$$ mod 2 eq 0\" of type Q{http://www.example.com/my-schema}foo: [jerr:JNTY0004]: cannot atomize an object: an atomic value is expected here"
}
