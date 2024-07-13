#!/usr/bin/env php
<?php

//read content of file ./feed.txt
$feed = file_get_contents('./feed.txt');

//check if read was successful
if ($feed === false) {
    $feed = "Error reading feed file";
}

$content_size = strlen($feed);

echo "Content-Type: text/plain\r\n";
echo "Content-Length: $content_size\r\n\r\n";
echo $feed;

?>