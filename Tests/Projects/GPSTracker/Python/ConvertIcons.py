from pathlib import Path
#import matplotlib
from PIL import Image
import numpy as np

def ProcessImage( _image ):

	# S'assurer qu'on est bien en RGB 24 bits
	_image = _image.convert( "RGB" )

	# Convert into NumPy array
	rgb = np.array(_image)

#	print( rgb.shape )
# (hauteur, largeur, 3)

	H, W, _ = rgb.shape
	if ( W != 64 or H != 64 or rgb.shape[2] != 3 ):
		raise "Wrong shape!"

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

	# Convert back into PIL image
	out = Image.fromarray(rgb.astype(np.uint8), "RGB")

	# Scale by 1:4
	out = out.resize( (W // 4, H // 4), Image.Resampling.LANCZOS )

	return out

def ProcessFolder( _source, _dest ):
	_dest.mkdir( exist_ok=True )

	for sourceFileName in _source.glob( "*.bmp" ):
		targetFileName = _dest / sourceFileName.name

		img = Image.open( sourceFileName )
		out = ProcessImage( img )

		out.save( targetFileName )

# Sauvegarder en BMP
#img = Image.open( "../Resources/Icons/Battery Charge/charge0.bmp" )
#out = ProcessImage( img )
#out.save( "output.bmp" )

ProcessFolder( Path( "../Resources/Icons/Battery Charge/" ), Path( "../Resources/Processed Icons/Battery Charge/" ) )
ProcessFolder( Path( "../Resources/Icons/Signal Strength/" ), Path( "../Resources/Processed Icons/Signal Strength/" ) )
ProcessFolder( Path( "../Resources/Icons/Wifi Strength/" ), Path( "../Resources/Processed Icons/Wifi Strength/" ) )

#	w, h = img.size
#
#	img_small = img.resize((w // 2, h // 2))
#
#	out = dst / file.name
#
#	img_small.save(out)

print( "Prout!" )
