<?php

// echo "<pre>";
// var_dump($_REQUEST);
// var_dump($_GET);
// var_dump($_POST);
// var_dump($_FILES);
// var_dump($_SERVER);
// echo "</pre>";
?>
<?php
echo "<h1>File Upload</h1>";
ini_set('display_errors', 1);
error_reporting(E_ALL);
if (isset($_FILES['file-to-upload']) && $_FILES['file-to-upload']['error'] === UPLOAD_ERR_OK) {
   
    $tmpName = $_FILES['file-to-upload']['tmp_name'];
    $fileName = basename($_FILES['file-to-upload']['name']);
    // if the file is an image
    if (preg_match('/^image\//', $_FILES['file-to-upload']['type'])) {
        $destDirectory = '../data/images/';
    } else {
        $destDirectory = '../data/upload/';
    }

    $destPath = $destDirectory . $fileName;



    // create the destination directory if it doesn't exist
    if (!is_dir($destDirectory)) {
        mkdir($destDirectory, 0777, true);
    }

    if (move_uploaded_file($tmpName, $destPath)) {
        echo "<h2> ✅ upload success $fileName</h2>";
        echo "Filename : $fileName<br>";
        echo "Temporary name : $tmpName<br>";
        echo "Path destination : $destPath<br>";
        echo "size : " . $_FILES['file-to-upload']['size'] . "<br>";
        echo "Type MIME : " . $_FILES['file-to-upload']['type'] . "<br>";
        echo "Error : " . $_FILES['file-to-upload']['error'] . "<br>";
        echo "Directory : $destDirectory<br>";
        echo "<p style='color: green;'>Thanks for your upload !</p>";
    } else {
        echo "<p> ❌ Error while uploading the file.</p>";  
    }
} else {
    echo "<p>❌ Error during file upload.</p>";
    echo "<p>Please check that the file is selected correctly.</p>";
}
?>