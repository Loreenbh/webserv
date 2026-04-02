<?php

$directory = '../data/upload';

echo "<body>";

if ($_SERVER['REQUEST_METHOD'] === 'DELETE') {

    parse_str($_SERVER['QUERY_STRING'], $params);
    $file = $params['file-to-delete'] ?? '';
    
    echo "<h1>Delete</h1>";
    echo "<p> file to delete : $file\n </p>";

    $safeFile = basename($file); 
    $file = $directory . '/' . $safeFile;
    echo "<p> Path : $file\n </p>";

    if (file_exists($file)) {
        if (is_file($file) && is_writable($file)&& is_readable($file)&&unlink($file)) {
            http_response_code(200); // OK
            echo "<p>✅ File deleted successfully.</p>";
        } else {
            http_response_code(500);
            echo "<p>❌ Error deleting file.</p>";
        }
    } else {
        http_response_code(404);
        echo "<p>❌ the file does not exist.</p>";
    }
} else {
    http_response_code(405); 
    echo "<p>❌ Method Not Allowed</p>";
}
echo "</body>";