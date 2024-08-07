#!/usr/bin/env php-cgi
<?php
// Example query string
$query_string = getenv('QUERY_STRING');

// Parse the query string into an associative array
parse_str($query_string, $query_params);

// Access individual variables
$seconds = isset($query_params['seconds']) ? (int)$query_params['seconds'] : 0;

// Validate the input
if ($seconds < 0) {
    $seconds = 0;
}

// Sleep for the specified number of seconds
sleep($seconds);

// Output the result
$message = "The wait is over! $seconds seconds of your life are gone, Congrats!";
// $message = "Content-Type: text/plain\r\n\r\nThe wait is over! $seconds seconds of your life are gone, Congrats!";

$length = strlen($message);

header('Content-Type: text/html; charset=utf-8');
echo($message)
?>
