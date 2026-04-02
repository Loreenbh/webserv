<?php 
echo "<h1>Inscription</h1>";
ini_set('display_errors', 1);
error_reporting(E_ALL);

// echo "<pre>";
// var_dump($_REQUEST);
// var_dump($_GET);
// var_dump($_POST);
// var_dump($_FILES);
// var_dump($_SERVER);
// echo "</pre>";

// check if the form is submitted
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    // get the form data
    $username = $_POST['username'] ?? '';
    $password = $_POST['password'] ?? '';
    $email = $_POST['email'] ?? '';

    // validate the form data
    if (empty($username) || empty($password) || empty($email)) {
        echo "<p style='color: red;'>Please fill in all fields.</p>";
    } else {
        // display the form data
        echo "<h2>Form Data</h2>";
        echo "<p>Username : $username</p>";
        echo "<p>Password : $password</p>";
        echo "<p>Email : $email</p>";
        echo "<p style='color: blue;'>Thank you for registering!</p>";
    }
}