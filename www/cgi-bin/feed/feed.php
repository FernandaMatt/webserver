#!/usr/bin/env php-cgi
<?php

// Define the path to the thoughts file
$thoughtsFile = 'thoughts.txt';

$default_toughts_file = 'thoughts_default.txt';

$new_thought = '';

if ($_SERVER['REQUEST_METHOD'] === 'POST') {

    // Check if the clear thoughts button was clicked
    if (isset($_POST['clearThoughts'])) {
        // Check if the file exists and is writable, if so, clear the file
        if (file_exists($thoughtsFile) && is_writable($thoughtsFile))
            file_put_contents($thoughtsFile, '');
    }

    // Check if the new thought is set and not empty
    else if (isset($_POST['newThought']) && $_POST['newThought'] !== '') {
        $new_thought = $_POST['newThought'];

        if ($new_thought === 'POST_DEFAULT') {
            if (file_exists($default_toughts_file) && is_readable($default_toughts_file))
                file_put_contents($thoughtsFile, file_get_contents($default_toughts_file));
        }
        else if ($new_thought !== '' && file_exists($thoughtsFile) && is_writable($thoughtsFile)) {
            // Add the new thought to the beginning of the thoughts file
            file_put_contents($thoughtsFile, "- \"" . $new_thought . "\"\n" . file_get_contents($thoughtsFile));
        }
    }
}

// Initialize an array to hold the thoughts
$thoughts = [];

// Check if the file exists and is readable, if so, read the file into the array
if (file_exists($thoughtsFile) && is_readable($thoughtsFile)) {
    $thoughts = file($thoughtsFile, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
}

header('Content-Type: text/html; charset=utf-8');

// Start outputting the HTML content
echo <<<HTML
<!DOCTYPE html>
<html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <link href="https://fonts.googleapis.com/css2?family=Roboto:wght@700&display=swap" rel="stylesheet">
        <link href="https://fonts.googleapis.com/css2?family=Edu+AU+VIC+WA+NT+Hand:wght@400..700&display=swap" rel="stylesheet">
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
            font-family: "Edu AU VIC WA NT Hand", cursive;
            font-weight: bold;
            font-style: italic;
            
        }
        #thoughts {
            margin-top: 40px; /* Adjusted margin for spacing */
            margin-bottom: 40px; /* Added margin for spacing */
            text-align: center;
            color: rgb(253, 243, 208);
            font-family: "Edu AU VIC WA NT Hand", cursive;
            font-size: 1.3em;
            font-weight: 500;
            font-style: italic;
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
        .round-button {
            background-color: rgb(251, 214, 173);
            color: rgb(11, 45, 11);
            border: none;
            border-radius: 12px;
            width: 160px;
            height: 40px;
            font-size: 1em;
            font-weight: bolder;
            cursor: pointer;
            outline: none;
            text-align: center;
        }
        .round-button:hover {
            background-color: rgb(238, 177, 108);
        }
		.thought-input {
			width: 300px;
			height: 25px;
			font-size: 1em;
			text-align: center;
            border-radius: 12px;
			margin-left: 10px;
			margin-right: 10px;
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
            <a href="/error_pages">Auto Index</a>
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
    <div>
        <form action="" method="post">
            <input type="text" name="newThought" placeholder="Enter your thought here..." class="thought-input">
            <input type="submit" value="Post Thought" class="round-button">
        </form>
    </div>
    <div>
        <form action="" method="post">
            <input type="submit" value="Clear Thoughts" name="clearThoughts" class="round-button">
        </form>
    </div>
</body>
</html>
HTML;
?>