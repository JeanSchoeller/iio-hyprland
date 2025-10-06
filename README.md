# iio-hyprland
A fork of okeri/iio-sway for Hyprland

Listens to iio-sensor-proxy and automatically changes Hyprland output orientation

## Installing 

:warning: Make sure [iio-sensor-proxy](https://gitlab.freedesktop.org/hadess/iio-sensor-proxy/) running :warning:

### Arch linux

`yay iio-hyprland-git`

`paru iio-hyprland-git`


### Nix/NixOS linux

To install locally

```sh
nix profile install github:JeanSchoeller/iio-hyprland
```

If you are using flakes to setup your system :

```nix
{
  inputs.iio-hyprland.url = "github:JeanSchoeller/iio-hyprland";
  outputs = {...}@inputs:{};
}
```

And add it to where you defined your packages :

```nix
{inputs, pkgs, ...}:{
  # ...
  environment.systemPackages = with pkgs; [
    inputs.iio-hyprland.packages.${pkgs.system}.default
  ]
  # ...
}
```

### Build from scratch

```
git clone https://github.com/JeanSchoeller/iio-hyprland

cd iio-hyprland

sudo make install
```

#### Uninstalling 
```
cd iio-hyprland

sudo make uninstall
```

## Running
`iio-hyprland [master window location] [monitor to rotate, default=eDP-1]`, run `hyprctl monitors` to list available outputs. Use either `--left-master` or `--right-master` to set the master window location to the left/top or right/bottom, leave blank to not adjust window layout on rotate. 

Add `exec-once = iio-hyprland` to `~/.config/hypr/hyprland.conf`

Some users reported that specifying the monitor in hyprland.conf could be necessary. For example, on Surface Pro:

`monitor=eDP-1,preferred,auto,2,transform,0`

In some devices, the display orientation may not match the accelerometer orientation (such as on the GPD Pocket series and others). It is possible to set the transform orientation using the `--transform 0,1,2,3` or for GPD Pocket (as an example) `--transform 3,0,1,2` These correspond to the Orientation ENUM (Normal, LeftUp, BotomUp, RightUp ) related to the hyprland transform property. 

## Touch rotation support

Should automatically rotate all Tablets and Touch Devices from `hyprctl devices`.
Thank you to Desktop31 for fetching the `hyprctl devices` output.

## Collaborators

[<img src="https://github.com/ForgotMyPasswd.png" width="30px;"/>](https://github.com/{{ForgotMyPasswd}}) ForgotMyPasswd

[<img src="https://github.com/Boom-Hacker.png" width="30px;"/>](https://github.com/{{Boom-Hacker}}) Boom-Hacker

[<img src="https://github.com/davydotcom.png" width="30px;"/>](https://github.com/{{davydotcom}}) davydotcom

