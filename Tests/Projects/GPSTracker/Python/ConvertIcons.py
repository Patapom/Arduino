import matplotlib
from PIL import Image
import numpy as np

img = Image.open( "../Resources/Icons/Battery Charge/charge0.bmp" )

# S'assurer qu'on est bien en RGB 24 bits
img = img.convert( "RGB" )

# Convertir en tableau NumPy
rgb = np.array(img)

print( rgb.shape )
# (hauteur, largeur, 3)

H, W, _ = rgb.shape
if ( W != 64 or H != 64 or rgb.shape[2] != 3 ):
	raise "Wrong shape!";

# Invert grayscale values so we have a black background
DIST_THRESHOLD = 0.05 * 255
for y in range(H):
	for x in range(W):
		r, g, b = rgb[y, x]
		gray = 0.3 * r + 0.5 * g + 0.2 * b
		sqDistanceFromGray = (r - gray)**2 + (g - gray)**2 + (b - gray)**2
		if ( sqDistanceFromGray < DIST_THRESHOLD*DIST_THRESHOLD ):
			# Invert only grayscale colors
			r = 255 - r
			g = 255 - g
			b = 255 - b

		rgb[y, x] = [r, g, b]


#rgb = 255 - rgb	# Exemple : inverser les couleurs

# Reconvertir en image PIL
out = Image.fromarray(rgb.astype(np.uint8), "RGB")

# Scale by 1:4
out = out.resize( (W // 2, H // 2), Image.Resampling.LANCZOS )

# Sauvegarder en BMP
out.save( "output.bmp" )

print( "Prout!" )
