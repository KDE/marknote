#  <img src="https://invent.kde.org/mbruchert/marknote/-/raw/master/logo.png">  Marknote

A simple markdown note management app

most formating features not supported yet

<img src="https://i.imgur.com/tJba9pK.png.png"  height="350" > <img src="https://i.imgur.com/9DsuKFP.png"  height="350" >

## build instructions

### flatpak builder (with kde sdk)
```
flatpak-builder tmp --force-clean --ccache --install --user org.kde.marknote.json
```
### cmake
```
mkdir build
cd build
cmake ..
make
```
