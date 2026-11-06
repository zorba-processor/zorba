copy $c := <a><b/></a> modify delete node $c/b return (count($c/*), 1 + 1)
