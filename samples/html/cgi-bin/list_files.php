<?php
header('Content-Type: application/json');
header('Connection: keep-alive');

$directory = '../data/upload';
if (!is_dir($directory)) {
    http_response_code(500);
    echo json_encode(["error" => "Dossier '$directory' introuvable"]);
    exit;
}

$files = scandir($directory);

// Garder seulement les fichiers
$files = array_filter($files, function($file) use ($directory) {
    return is_file($directory . '/' . $file);
});

$files = array_values($files);

// Ne pas ajouter le chemin, garder juste le nom
$files = array_filter($files, function($file) use ($directory) {
    return is_readable($directory . '/' . $file);
});

echo json_encode($files);
?>
