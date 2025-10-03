{
  lib,
  stdenv,
  fetchFromGitHub,
  meson,
  ninja,
  dbus,
  pkg-config,
  jq,
  makeWrapper
}:
stdenv.mkDerivation rec {
  pname = "iio-hyprland";
  version = "unstable-2023-09-27";

  src = lib.cleanSource ./.;

  nativeBuildInputs = [
    pkg-config
    meson
    ninja
    makeWrapper
  ];

  buildInputs = [
    dbus
    jq
  ];

  postInstall = ''
    wrapProgram "$out/bin/iio-hyprland" \
      --prefix PATH : "${jq}/bin"
  '';

  meta = with lib; {
    description = "Listen iio-sensor-proxy and auto change Hyprland output orientation";
    homepage = "https://github.com/JeanSchoeller/iio-hyprland";
    license = licenses.gpl3Only;
    maintainers = with maintainers; [ ];
    mainProgram = "iio-hyprland";
    platforms = platforms.all;
  };
}
