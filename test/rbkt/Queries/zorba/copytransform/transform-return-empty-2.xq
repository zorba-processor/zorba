declare function local:f($n)
{
  copy $c := $n modify delete node $c/b return ()
};

count(local:f(<a><b/></a>)), count(for $i in 1 to 3 return copy $c := <a/> modify () return ())
