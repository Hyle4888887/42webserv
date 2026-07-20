<?php
header("Content-Type: text/html; charset=UTF-8");
?>
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>CGI PHP — Info</title>
  <style>
    body { font-family: sans-serif; max-width: 700px; margin: 60px auto; color: #222; background: #f9f9f9; }
    table { border-collapse: collapse; width: 100%; margin-top: 16px; }
    th, td { border: 1px solid #ddd; padding: 8px 12px; text-align: left; }
    th { background: #f0f0f0; }
    a { color: #0066cc; }
    .version { background: #cce5ff; border: 1px solid #99caff; border-radius: 4px; padding: 10px 16px; display: inline-block; margin-bottom: 16px; }
  </style>
</head>
<body>
  <h1>⚙️ CGI PHP — Info</h1>
  <p>Ce script CGI tourne côté serveur via <strong>php-cgi</strong>.</p>

  <div class="version">
    PHP version : <strong><?php echo phpversion(); ?></strong>
  </div>

  <h2>Variables d'environnement CGI</h2>
  <table>
    <tr><th>Variable</th><th>Valeur</th></tr>
    <?php
    $vars = array(
        "REQUEST_METHOD", "SCRIPT_FILENAME", "PATH_INFO",
        "QUERY_STRING", "CONTENT_TYPE", "CONTENT_LENGTH",
        "SERVER_NAME", "SERVER_PORT", "HTTP_HOST", "REDIRECT_STATUS"
    );
    foreach ($vars as $v) {
        $val = isset($_SERVER[$v]) ? htmlspecialchars($_SERVER[$v]) : "<non définie>";
        echo "    <tr><td><code>$v</code></td><td>$val</td></tr>\n";
    }
    ?>
  </table>

  <h2>Date/heure serveur</h2>
  <p><?php echo date('Y-m-d H:i:s'); ?></p>

  <p style="margin-top:30px"><a href="/index.html">← Retour à l'accueil</a></p>
</body>
</html>
