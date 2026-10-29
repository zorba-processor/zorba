(: Ensure that disabling HTTP URI resolution doesn't defeat fn:doc(). :)
fn:doc("http://$HTTP_TEST_SERVER/cgi-bin/test-xml?querystring")/body/query
