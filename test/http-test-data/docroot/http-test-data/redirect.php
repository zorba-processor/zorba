<?php
// Redirects to the URL given in the "to" parameter (security policy tests:
// redirects out of an allowed prefix, to other hosts or protocols).
$to = isset($_GET['to']) ? $_GET['to'] : '/';
header('Location: ' . $to, true, 302);
