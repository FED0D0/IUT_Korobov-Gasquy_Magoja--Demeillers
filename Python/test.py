import cv2
import numpy as np
from matplotlib import pyplot as plt  # <-- L'importation manquante était ici
from urllib.request import urlopen

# 1. Chargement de l'image du TP depuis l'URL de Valentin Gies
req = urlopen("http://www.vgies.com/downloads/robocup.png")
arr = np.asarray(bytearray(req.read()), dtype=np.uint8)
img = cv2.imdecode(arr, -1)

if img is None:
    print("Erreur : Image introuvable. Vérifiez le chemin ou la connexion internet.")
    exit()

# 2. Conversion vers l'espace HSV et décomposition des canaux
img_hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
h, s, v = cv2.split(img_hsv)

# 3. Égalisation des histogrammes de chacune des composantes
Heq = cv2.equalizeHist(h)
Seq = cv2.equalizeHist(s)
Veq = cv2.equalizeHist(v)

# 4. Recombinaison en une image HSV et conversion en BGR pour affichage
hsv_egalise = cv2.merge([Heq, Seq, Veq])
img_resultat_bgr = cv2.cvtColor(hsv_egalise, cv2.COLOR_HSV2BGR)

# --- FONCTION POUR REUTILISER L'AFFICHAGE DE L'HISTOGRAMME ---
def afficher_histogramme(canal_data, titre, couleur_courbe):
    """Calcule et affiche l'histogramme et la CDF d'un canal."""
    # Calcul de l'histogramme
    hist, bins = np.histogram(canal_data.flatten(), 256, [0, 256])
    # Calcul de la CDF (courbe cumulée)
    cdf = hist.cumsum()
    cdf_normalized = cdf * float(hist.max()) / cdf.max()
    
    # Configuration du graphique Matplotlib
    plt.figure(titre)
    plt.plot(cdf_normalized, color='b', label='CDF (Cumulé)')
    plt.hist(canal_data.flatten(), 256, [0, 256], color=couleur_courbe, label='Histogramme')
    plt.xlim([0, 256])
    plt.title(titre)
    plt.legend(loc="upper left")

# 5. Affichage des histogrammes AVANT égalisation
afficher_histogramme(h, "Canal H (Teinte) - Original", 'r')
afficher_histogramme(s, "Canal S (Saturation) - Original", 'g')
afficher_histogramme(v, "Canal V (Valeur/Luminosité) - Original", 'gray')

# 6. Affichage des histogrammes APRÈS égalisation
afficher_histogramme(Heq, "Canal H (Teinte) - Égalisé", 'r')
afficher_histogramme(Seq, "Canal S (Saturation) - Égalisé", 'g')
afficher_histogramme(Veq, "Canal V (Valeur/Luminosité) - Égalisé", 'gray')

# 7. Visualisation des images avec OpenCV
cv2.imshow("1. Image Originale BGR", img)
cv2.imshow("2. Image Finale Recombinée HSV Égalisée", img_resultat_bgr)

# Déclenchement de l'affichage de tous les graphiques Matplotlib
plt.show()

cv2.waitKey(0)
cv2.destroyAllWindows()
