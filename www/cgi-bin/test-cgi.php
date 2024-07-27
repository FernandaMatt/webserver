#!/usr/bin/env php-cgi
<?php

// Read the raw POST data from php://input
$request_body = file_get_contents('php://input');

$message = "Hello, World! Atenção: Aqui é Brasil!";

$length = strlen($message);

header("Content-Type: text/plain");
echo $message;

?>