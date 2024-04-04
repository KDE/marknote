#  <img src="https://invent.kde.org/mbruchert/marknote/-/raw/master/logo.png">  MarkNote

Write down your thoughts.

Marknote supports a wide range of formating options usefully for taking every day notes, like bold, italic, underlined and strike through fonts as well as headings, lists, check boxes, images and more.

<img src="https://i.imgur.com/tJba9pK.png.png"  height="350" > <img src="https://i.imgur.com/9DsuKFP.png"  height="350" >

## Installation

<a href='https://flathub.org/apps/details/org.kde.marknote'><img width='190px' alt='Download on Flathub' src='https://flathub.org/assets/badges/flathub-badge-i-en.png'/></a>

## Build Instructions

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
### kdesrc-build
```
kdesrc-build marknote
```
