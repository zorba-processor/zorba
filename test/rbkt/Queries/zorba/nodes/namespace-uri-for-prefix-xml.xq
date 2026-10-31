(: The xml prefix is in scope for every element; fn:namespace-uri-for-prefix
   returns an xs:anyURI (F&O 3.0 11.2.5, 11.1.1; XQuery 3.0 2.1.1). :)
let $e := <e xmlns:p="urn:p"/>
let $xml := namespace-uri-for-prefix("xml", $e)
return (string($xml),
        ($xml, "x")[1] instance of xs:anyURI,
        (namespace-uri-for-prefix("p", $e), "x")[1] instance of xs:anyURI,
        string(namespace-uri-from-QName(resolve-QName("xml:a", $e))))
