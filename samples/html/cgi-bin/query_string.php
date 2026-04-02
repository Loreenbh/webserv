
<?php

$directory = '../data/upload/';

$word = $_GET['word'] ?? 'Lorem';
$color = $_GET['color'] ?? 'red';
$file = $_GET['file'] ?? 'sample.txt';
$file = $directory . basename($file); 

if (!file_exists($file)) {
    echo "<h1>Make sure the file exists.</h1>";
    exit;
}

if (empty($word)) {
    echo "<h1>Please provide a word.</h1>";
    exit;
}

$word = preg_replace('/\b(?:if|else|while|for|foreach|switch|case|break|continue|return|function|class|public|private|protected|static|const|var)\b/', '', $word);

$word = preg_replace('/<[^>]+>/', '', $word);


if (empty($color)) {
    echo "<h1>Please provide a color.</h1>";
    exit;
}

if (empty($file)) {
    echo "<h1>Please provide a file to read.</h1>";
    exit;
}



function highlightWord($text, $word, $color) {
    return preg_replace("/($word)/i", "<span style='color: $color;'>$1</span>", $text);
}


$html_content = file_get_contents($file);



$html_content = highlightWord($html_content, $word, $color);
?>
<?php
echo $html_content;
?>