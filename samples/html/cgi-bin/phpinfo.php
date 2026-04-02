
<?php
ob_start();
phpinfo();
$phpinfo = "<!DOCTYPE html> ";
$phpinfo .= "<html lang='en'>\n";
$phpinfo .= "<head>\n";
$phpinfo .= "<meta charset='UTF-8'>\n";
$phpinfo .= "<meta name='viewport' content='width=device-width, initial-scale=1.0'>\n";
$phpinfo .= "<meta name='description' content='PHP Info Page'>\n";
$phpinfo .= "<title>PHP Info</title>\n";

$phpinfo .= ob_get_clean();


// create a custom style for the phpinfo output
$style = '<style>
    body { font-family: Arial, sans-serif; background-color: #f4f4f4; margin: 0; padding: 20px; }
    .phpinfo { max-width: 90%; margin: auto; background: #fff; padding: 20px; border-radius: 10px; box-shadow: 0px 0px 10px rgba(0, 0, 0, 0.1); }
    table { width: 100%; border-collapse: collapse; margin-top: 10px; }
    th, td { padding: 10px; border: 1px solid #ddd; text-align: left; }
    th { background-color: #555; color: white; }
    h1 { text-align: center; color: #333; }
</style>';

// add the custom style to the phpinfo output
$phpinfo = preg_replace('/<body>/', '<body><div class="phpinfo">' . $style, $phpinfo);
$phpinfo = preg_replace('/<\/body>/', '</div></body>', $phpinfo);
// add html standard mode to the phpinfo output
$phpinfo .= "</html>";


echo $phpinfo;
?>
