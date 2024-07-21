#!/usr/bin/env php-cgi
<?php

$message = "Hello, World! Atenção: Aqui é Brasil!";

$length = strlen($message);

header("Content-Type: text/plain");
echo $message;

?>