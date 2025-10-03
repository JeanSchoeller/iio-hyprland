{
  lib,
  stdenv,
  fetchFromGitHub,
  meson,
  ninja,
  dbus,
  pkg-config,
  jq,
}:
stdenv.mkDerivation rec {
  pname = "iio-hyprland";
  version = "unstable-2023-09-27";

  src = lib.cleanSource ./.;

  nativeBuildInputs = [
    pkg-config
    dbus
    meson
    ninja
    jq
  ];

  meta = with lib; {
    description = "Listen iio-sensor-proxy and auto change Hyprland output orientation";
    homepage = "https://github.com/JeanSchoeller/iio-hyprland";
    license = licenses.gpl3Only;
    maintainers = with maintainers; [ ];
    mainProgram = "iio-hyprland";
    platforms = platforms.all;
  };
}
