<?php

header('Content-Type: application/json');

$filename = '../data/upload/' . basename($_GET['file-to-download']);


if (empty($filename) || !preg_match('/^[a-zA-Z0-9_\-\.]+$/', basename($_GET['file-to-download']))) {
    http_response_code(400);
    echo json_encode(["error" => "Invalid filename."]);
    exit;
}


if (strpos(realpath($filename), realpath('../data/upload/')) !== 0) {
    http_response_code(403);
    echo json_encode(["error" => "❌ Access denied."]);
    exit;
}


if (file_exists($filename)) {
    $fileContent = file_get_contents($filename);
    $base64 = base64_encode($fileContent);
    $mimeType = mime_content_type($filename);
    
    echo json_encode([
        "fileName" => basename($filename),
        "mimeType" => $mimeType,
        "base64" => $base64,
        "message" => "✅ File downloaded successfully."
    ]);
    exit;
} else {
    http_response_code(404);
    echo json_encode(["error" => "❌ File not found."]);
}
?>
