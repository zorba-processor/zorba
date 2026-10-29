<?php
// Target of the /remotequeue redirect (Apache answers /remotequeue with a
// 301 to /remotequeue/). headers-on-redirect checks this exact header value.
header("Content-Type: text/html;charset=UTF-8");
?>
<!DOCTYPE html>
<html><head><title>redirect target</title></head><body></body></html>
