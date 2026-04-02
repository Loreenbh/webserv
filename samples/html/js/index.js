
function loadFiles() {
	console.log('Début du chargement des fichiers');

	fetch('./cgi-bin/list_files.php')
		.then(function(response) {
			return response.json();
		})
		.then(function(files) {
			const select = document.getElementById('file-select');
			select.innerHTML = ''; // Vider la liste avant de recharger
			const selectDelete = document.getElementById('file-select-delete');
			selectDelete.innerHTML = ''; // Vider la liste avant de recharger
			const selectDownload = document.getElementById('file-select-download');
			selectDownload.innerHTML = ''; // Vider la liste avant de recharger
			for (var i = 0; i < files.length; i++) {
				var file = files[i];
				var option = document.createElement('option');
				option.value = file;
				option.textContent = file;
				if (file.endsWith('.txt')) {
					select.appendChild(option);
				}
				selectDelete.appendChild(option.cloneNode(true));
				selectDownload.appendChild(option.cloneNode(true));
				console.log(file);
			}

			if (files.length > 0) {
				selectDelete.value = files[0];
				selectDownload.value = files[0];
			}

			console.log('Fichiers chargés avec succès');
		})
		.catch(function(error) {
			console.error('Erreur lors du chargement des fichiers :', error);
		});
}

function deleteFile() {
	const select = document.getElementById('file-select-delete');
	const fileName = select.value;
	const iframe = document.getElementById('iframe-result');
	if (fileName) {
		fetch('/test-delete?file-to-delete=' + fileName, {
			method: 'DELETE',
		})
			.then(function(response) {
				if (response.ok) {
					alert('Fichier supprimé avec succès : ' + fileName);
					loadFiles();
				} else {
					alert('Erreur lors de la suppression du fichier : ' + fileName);
				}
				return response.text();
			})
			.then(function(data) {
				console.log('Réponse du serveur :', data);
				iframe.src = 'data:text/html;charset=utf-8,' + encodeURIComponent(data);
			})
			.catch(function(error) {
				console.error('Erreur lors de la suppression du fichier :', error);
			});
	}
}

function downloadFile() {
	const select = document.getElementById('file-select-download');
	const fileName = select.value;
	const iframe = document.getElementById('iframe-result');

	if (fileName) {
		fetch('/test-download?file-to-download=' + encodeURIComponent(fileName))
			.then(response => {
				if (!response.ok) {
					throw new Error('Erreur HTTP : ' + response.status);
				}
				return response.json();
			})
			.then(data => {
				if (data.error) {
					throw new Error(data.error);
				}

				// Décode le fichier base64 en Blob
				const byteCharacters = atob(data.base64);
				const byteArrays = [];

				for (let i = 0; i < byteCharacters.length; i += 512) {
					const slice = byteCharacters.slice(i, i + 512);
					const byteNumbers = new Array(slice.length);
					for (let j = 0; j < slice.length; j++) {
						byteNumbers[j] = slice.charCodeAt(j);
					}
					const byteArray = new Uint8Array(byteNumbers);
					byteArrays.push(byteArray);
				}

				const blob = new Blob(byteArrays, { type: data.mimeType });
				const url = URL.createObjectURL(blob);

				// Crée un lien pour déclencher le téléchargement
				const a = document.createElement('a');
				a.href = url;
				a.download = data.fileName;
				document.body.appendChild(a);
				a.click();
				a.remove();

				URL.revokeObjectURL(url);

				// Affiche un message dans l'iframe
				const message = data.message || 'Fichier téléchargé avec succès.';
				iframe.src = 'data:text/html;charset=utf-8,' + encodeURIComponent(`<p>${message}</p>`);
			})
			.catch(error => {
				console.error('Erreur lors du téléchargement :', error.message);
				iframe.src = 'data:text/html;charset=utf-8,' + encodeURIComponent(`<p style="color:red;">${error.message}</p>`);
			});
	}
}



function loadAlbum() {
    const iframe = document.getElementById('iframe-result');
    iframe.style.display = 'block';
    iframe.src = 'gallery.html';

    // Quand l'iframe a fini de charger, appelle la fonction dans son contexte
    iframe.onload = () => {
        if (iframe.contentWindow.loadImages) {
            iframe.contentWindow.loadImages(); // Appelle loadImages() définie dans gallery.html
        } else {
            console.error("loadImages() n'est pas définie dans gallery.html");
        }
    };
}



// Exécuter au chargement de la page
window.addEventListener('DOMContentLoaded', loadFiles);

// Exécuter après envoi du formulaire
document.getElementById('uploadForm').addEventListener('submit', function(e) {
	setTimeout(loadFiles, 1000); // attendre un peu pour que le fichier soit bien enregistré
});
