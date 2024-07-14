#!/usr/bin/env php
<?php

$message = "Hello, World!";

$length = strlen($message);

echo "Content-Type: text/plain\r\n";
echo "Content-Length: $length\r\n\r\n";
echo $message;

?>