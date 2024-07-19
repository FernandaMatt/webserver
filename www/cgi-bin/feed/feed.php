#!/usr/bin/env php-cgi
<?php
header('Content-Type: text/html; charset=utf-8');

// Define the path to the thoughts file
$thoughtsFile = 'thoughts.txt';

// Initialize an array to hold the thoughts
$thoughts = [];

// Check if the file exists and is readable
if (file_exists($thoughtsFile) && is_readable($thoughtsFile)) {

    $request_body = file_get_contents('php://input');
    // $request_body = file_get_contents('php://stdin');

    //add the request body at the end of the thoughts file
    file_put_contents($thoughtsFile, "\n$request_body", FILE_APPEND);


    // file_put_contents($thoughtsFile, $request_body);

    if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    
        // Check if the new thought is set and not empty
        if (isset($_POST['newThought']) && $_POST['newThought'] !== '') {
            // Add the new thought to the array
            $thoughts[] = $_POST['newThought'];
    
            // Write the thoughts back to the file
            file_put_contents($thoughtsFile, implode("\n", $thoughts));
        }
    }
    // Read the file line by line into the array
    $thoughts = file($thoughtsFile, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
}

//Check request method

// Start outputting the HTML content
echo <<<HTML
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link href="https://fonts.googleapis.com/css2?family=Roboto:wght@700&display=swap" rel="stylesheet">
    <title>Another Server</title>
    <style>
        html, body {
            height: 100%;
            margin: 0;
            overflow: hidden; /* Prevent scroll bars */
        }
        body {
            background-color: rgb(11, 45, 11);
            color: rgb(251, 214, 173);
            font-family: 'Roboto', sans-serif;
            align-items: center;
            justify-content: center;
        }
        header {
            position: absolute;
            top: 0;
            width: 100%;
            display: flex;
            justify-content: flex-end;
            padding: 10px 20px;
            box-sizing: border-box; /* Include padding in the width calculation */
        }
        nav a {
            color: rgb(251, 214, 173);
            text-decoration: none;
            margin-left: 20px;
            margin-right: 5px;
            font-size: 1.1em;
        }
        h1 {
            font-size: 3em;
            margin-top: 80px; /* Added margin to move the h1 below the header */
            text-align: center;
            
        }
        #thoughts {
            margin-top: 20px; /* Adjusted margin for spacing */
            text-align: center;
        }
        form {
            text-align: center;
        }
        input[type="text"] {
            width: 65%;
            padding: 10px;
            margin-top: 20px;
            text-align: center;
        }
        input[type="submit"] {
            padding: 10px 20px;
            margin-top: 10px;
        }
    </style>
</head>
<body>
    <header>
        <nav>
            <a href="/">Home Base</a>
            <a href="/timer">Wait!</a>
            <a href="/feed/feed.php">Feed</a>
            <a href="/wrongPath">Not Found</a>
            <a href="/error_pages">Forbidden</a>
        </nav>
    </header>
    <h1>Wall of Thoughts</h1>
    <div id="thoughts">
HTML;

// Display each thought
foreach ($thoughts as $thought) {
    echo "<p>" . htmlspecialchars($thought) . "</p>";
}

// Continue with the HTML for input and button
echo <<<HTML
    </div>
    <form action="" method="post">
        <input type="text" name="newThought" placeholder="Enter your thought here...">
        <input type="submit" value="Post Thought">
    </form>
</body>
</html>
HTML;
?>