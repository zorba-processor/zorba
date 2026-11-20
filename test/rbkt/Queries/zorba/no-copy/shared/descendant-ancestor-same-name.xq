(: A constructor that does not copy its content (the no-copy optimization at
   O1 and O2) gives a tree in which descendant steps find what they find in a
   copy: elements nested in an element of the same name. One shape per query,
   since an axis elsewhere in the query can make the constructor copy. :)

declare function local:leaf()
{
  <a>x</a>
};

count(<a><b>{ local:leaf() }</b></a>/descendant-or-self::a)
