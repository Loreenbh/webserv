# Modifications :
- Gestion du timeout, utilisation de <ctime>
- Les clients sont maintenant écoutés sur l'écriture et la lecture

# 🌍 Liste complète des variables d'environnement CGI

Voici la liste des variables d'environnement CGI que le serveur web crée pour un script CGI exécuté :

| **Variable**             | **Description**                                                                 |
|--------------------------|---------------------------------------------------------------------------------|
| `REQUEST_METHOD`          | Méthode HTTP utilisée (`GET`, `POST`, `PUT`, etc.).                             |
| `SCRIPT_FILENAME`         | Chemin absolu du fichier script CGI exécuté.                                    |
| `SCRIPT_NAME`             | Chemin du script depuis la racine du serveur (`/cgi-bin/script.php`).           |
| `QUERY_STRING`            | Paramètres de la requête GET (`name=John&age=30`).                             |
| `CONTENT_TYPE`            | Type de contenu des données envoyées (`application/x-www-form-urlencoded`, `multipart/form-data`). |
| `CONTENT_LENGTH`          | Taille des données envoyées dans une requête POST.                             |
| `GATEWAY_INTERFACE`       | Version de l'interface CGI (`CGI/1.1`).                                         |
| `SERVER_PROTOCOL`         | Version du protocole HTTP (`HTTP/1.1`, `HTTP/2`).                              |
| `SERVER_SOFTWARE`         | Nom du serveur web (`MyWebServer/1.0`).                                        |
| `SERVER_NAME`             | Nom de domaine ou adresse IP du serveur.                                       |
| `SERVER_PORT`             | Port sur lequel le serveur écoute (`80`, `443`).                               |
| `REMOTE_ADDR`             | Adresse IP du client qui fait la requête.                                      |
| `REMOTE_PORT`             | Port utilisé par le client.                                                    |
| `REMOTE_HOST`             | Nom de l'hôte du client (si disponible).                                       |
| `REDIRECT_STATUS`         | Statut utilisé pour certaines implémentations CGI (`200`).                     |
| `DOCUMENT_ROOT`           | Répertoire racine du serveur (`/var/www/html`).                                |
| `HTTP_HOST`               | Hostname spécifié par le client (`example.com`).                               |
| `HTTP_USER_AGENT`         | Navigateur utilisé (`Mozilla/5.0 ...`).                                        |
| `HTTP_REFERER`            | URL de la page précédente, si disponible.                                      |
| `HTTP_COOKIE`             | Cookies envoyés par le client.                                                 |
| `HTTP_ACCEPT`             | Types MIME acceptés par le client (`text/html, application/json`).              |
| `HTTP_ACCEPT_LANGUAGE`    | Langues acceptées (`fr-FR, en-US`).                                            |
| `HTTP_ACCEPT_ENCODING`    | Encodages acceptés (`gzip, deflate`).                                          |
| `PATH_INFO`               | Informations supplémentaires après le nom du script (`/extra/path`).          |
| `PATH_TRANSLATED`         | Chemin du fichier correspondant à `PATH_INFO`.                                 |
| `REQUEST_URI`             | URI complète demandée (`/index.php?name=John`).                               |
| `HTTPS`                   | Défini à `"on"` si la requête est en HTTPS, absent sinon.                      |

## Exemple de récupération des variables en PHP

Tu peux récupérer ces variables en utilisant le tableau `$_SERVER` en PHP. Exemple :

```php
<?php
echo "<pre>";
print_r($_SERVER);
echo "</pre>";
?>


