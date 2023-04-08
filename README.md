# Marknote
![](https://invent.kde.org/mbruchert/marknote/-/raw/master/logo.png)

A simple markdown note management app

most formating features not supported yet

![](https://i.imgur.com/tJba9pK.png)

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
