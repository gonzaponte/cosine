[![built with nix](https://builtwithnix.org/badge.svg)](https://builtwithnix.org)

# cosine

**CO**lina **SI**mulation **N**vironment is a Geant4/Nain4 simulation of the COLINA noble-liquid detector. It builds the detector geometry, materials, optical surfaces, field-cage components, and SiPM arrays; transports optical photons and calibration particles; and writes simulation data and run configuration to HDF5.

The project is a C++17 application built with Meson. The Nix flake supplies the compiler and the native dependencies, including Geant4, Nain4, HDF5/HighFive, Catch2, and the development tools.

## Quick start

Use Nix and `direnv`. Nix gives the project a reproducible Geant4 toolchain, while `direnv` loads that toolchain automatically whenever you enter the project directory.

### One-time machine setup

Install Nix with flakes enabled. We recommend the [Determinate Nix Installer](https://determinate.systems/nix-installer), which provides installation instructions for Linux, macOS, and WSL. Then install `direnv` using your operating system or Nix package manager. For a Nix installation, this is one possible command:

```sh
nix profile add nixpkgs#direnv
```

Enable the `direnv` hook in your shell.

```sh
# if you have a zsh shell
eval "$(direnv hook zsh)"

# if you have a bash shell
eval "$(direnv hook bash)"
```

You probably want to do it automatically when you start a new shell, if so, put your command in your shell configuration file:

```sh
# if you have a zsh shell
echo 'eval "$(direnv hook zsh)"' >> ~/.zshrc
# or echo 'eval "$(direnv hook zsh)"' >> ~/.zsh_profile

# if you have a bash shell
echo 'eval "$(direnv hook bash)"' >> ~/.bashrc
# or echo 'eval "$(direnv hook bash)"' >> ~/.bash_profile
```

The repository's `.envrc` bootstraps a pinned `nix-direnv` helper if one is not already available. No manual Geant4, HDF5, or compiler installation is needed.

### Checkout and activate the project

```sh
git clone https://github.com/colina-exp/cosine.git
cd cosine
direnv allow
```

The first activation may download the flake inputs and build the development environment, which can take a while (more than half an hour, easily). After that, entering the directory with `cd cosine` automatically loads the project setup (fast); `direnv` loads the default Clang development shell automatically.

If `direnv` is unavailable, the equivalent manual activation is:

```sh
nix develop .#clang
```

The flake also provides a GCC shell:

```sh
nix develop .#gcc
```

## Run a simulation

The `just` recipes hide the Meson configure/build/install steps. A useful first run is:

```sh
just run --beam-on 10 -e macs/s1.mac
```

This runs 10 Geant4 events using the S1 optical-photon configuration and writes `output_file.h5` in the current directory. `just run` rebuilds and installs the executable before running it, so changes to the source are picked up automatically.

The bare `just` command is shorthand for `just run --beam-on 10`. It does not select a source macro, and the empty configuration defaults to zero primary particles, so use one of the macros below for a meaningful simulation.

For an S2 simulation:

```sh
just run --beam-on 10 -e macs/s2.mac
```

To choose an output file and seed, pass Geant4 commands as quoted early commands:

```sh
mkdir -p results
just run --beam-on 10 \
  -e macs/s1.mac \
     "/sim/outputfile results/s1.h5" \
     "/sim/seed 42"
```

The same application can be run directly through the flake, without entering a development shell explicitly:

```sh
nix run . -- --beam-on 10 -e macs/s1.mac
```

Run `cosine --help` for the complete Nain4 command-line interface. The most useful options are:

| Option | Meaning |
| --- | --- |
| `-n`, `--beam-on N` | Run `N` Geant4 events. |
| `-e`, `--early ITEMS...` | Apply macro files or `/sim/...` commands before initialization. |
| `-l`, `--late ITEMS...` | Apply commands after the run manager/actions have been created. |
| `-g`, `--vis [ITEMS...]` | Enable the Geant4 GUI; `vis.mac` is the default visualization macro. |
| `-m`, `--macro-path PATHS...` | Add directories to the Geant4 macro search path. |
| `--save-rng DIR` / `--with-rng FILE` | Save or restore random-number-generator state. |

On Linux systems, `execute-with-nixgl-if-needed.sh` automatically uses `nixGL` when a graphical run requests `--vis` or `-g`. A working display/OpenGL environment is still required. Unfortunately, nixGL is very machine dependent and needs to go separate from the rest of the dependencies. The recommended way to install nixGL is

```sh
nix profile add github:nix-community/nixGL --impure
```

With nixGL you can run visualization. For example, an interactive visualization run is:

```sh
just run --beam-on 1 -e macs/s1.mac --vis
```

## Simulation configuration

The application exposes its configuration through Geant4's `/sim/` messenger. Settings can be placed in a macro file or supplied as early commands. Macro values are case-insensitive for enum-like settings.

Common controls include:

| Command | Values / purpose |
| --- | --- |
| `/sim/generator` | `debug`, `s1`, `s2`, `kr`, or `fe`. |
| `/sim/nparticles` | Number of primary particles generated per event. |
| `/sim/outputfile` | HDF5 output path; the file is overwritten on each run. |
| `/sim/seed` | Random seed. |
| `/sim/start_id` | Event-ID offset, useful when combining independent jobs. |
| `/sim/optical` | Enable or disable optical-photon transport. |
| `/sim/medium` | `xenon`, `argon`, or `argonxenon`; the krypton material is not implemented in the geometry yet. |
| `/sim/xenon_fraction` | Xenon fraction for an argon/xenon mixture. |
| `/sim/store_steps` | Store volume-crossing records. |
| `/sim/store_sens` | Store SiPM sensor hits. |
| `/sim/store_ihits` | Store deposited-energy/ionization hits. |
| `/sim/store_tracks` | Store track records. |
| `/sim/store_sources` | Store primary-source records. |
| `/sim/calib_belt` | `straight`, `spiral`, or `none`. |

The detector dimensions and optical configuration are also exposed through `/sim/`; `macs/colina.mac` and `macs/pcolina.mac` provide full-size and reduced-size parameter sets. The default geometry is constructed by `src/geometry/pcolina.cc` using the current `geometry_config` values.

The available generators are:

- `debug`: one or more geantinos at a fixed position, useful for checking volume crossings.
- `s1`: optical photons generated throughout the drift region.
- `s2`: optical photons generated in the electroluminescence region.
- `kr`: a fixed-energy electron calibration source.
- `fe`: an Fe-55 ion source; `macs/fe55.mac` also shows how to set a fixed generation vertex.

The program always loads `macs/physics_em_optical.mac` before applying command-line early settings. This installs the electromagnetic, optical, step-limiter, decay, and radioactive-decay physics used by the simulation.

## Included macro files

| File | Purpose |
| --- | --- |
| `macs/s1.mac` | S1 optical-photon run with SiPM/source output enabled. |
| `macs/s2.mac` | S2 optical-photon run with SiPM/source output enabled. |
| `macs/lt.mac` | Higher-statistics S2 light-table configuration. |
| `macs/fe55.mac` | Fe-55 source with ionization, track, sensor, and source output. |
| `macs/kr.mac` | Electron calibration configuration. |
| `macs/geantinos.mac` | Geantino volume-crossing/debug configuration. |
| `macs/colina.mac` | COLINA detector dimensions. |
| `macs/pcolina.mac` | Reduced detector dimensions for smaller or faster studies. |
| `macs/vis.mac`, `macs/vis2.mac` | Interactive visualization and color overrides. |

## HDF5 output

Runs write an HDF5 file with an `MC` group. Datasets are created for the enabled output streams:

```text
/MC/config
/MC/sources
/MC/sensor_hits
/MC/ionization_hits
/MC/tracks
/MC/volume_changes
```

`/MC/config` records the simulation and geometry parameters, including units, so a run can be interpreted from its output file. The other datasets contain source particles, SiPM detections, deposited energy, track summaries, and selected volume crossings respectively. Use the Nix-provided HDF5 tools to inspect a file:

```sh
h5dump -n output_file.h5
```

Output files use HDF5 overwrite mode. Set `/sim/outputfile` before a run if you want to preserve an earlier result.

## Build and test commands

Useful `just` recipes are:

```sh
just build        # configure and compile build/cosine
just install      # install the executable and library below install/cosine
just test         # build/install and run the Catch2 test suite
just test -v      # show the name of each test as it runs
just test -vv     # show full output for each test
just run-no-install --beam-on 10 -e macs/s1.mac
just clean        # remove build/ and install/
```

The test runner launches each Catch2 test case in a separate process because Geant4 owns substantial global state. `just catch2-demo` builds the small Catch2 demonstration program in `test/test-catch2-demo.cc`; that program intentionally contains examples of failing assertions.

For a package build managed entirely by Nix:

```sh
nix build .#
nix run . -- --beam-on 10 -e macs/s1.mac
```

## Parallel jobs and scans

`just parallelize` starts independent batch-mode processes, gives each job a different seed and event-ID range, and writes one HDF5 file plus one log per job. For example:

```sh
just parallelize 40 10000 0 xe_lt 123456789 \
  -e macs/colina.mac macs/lt.mac "/sim/medium xenon"
```

The arguments are `n_jobs`, `n_evt`, `first`, `pattern`, and `seed`. This example creates files such as `out/xe_lt_0000.h5` and logs such as `log/xe_lt_0000.log`; jobs are scheduled in batches of 12. It is a high-statistics workload, not a smoke test. The `s1` and `s2` recipes in `justfile` are larger predefined scans built on top of the same helper.

## Repository layout

```text
.envrc                 direnv entry point for the Nix development shell
flake.nix              pinned Nix development shells and packages
flake/                 flake outputs and shared shell setup
justfile               build, run, test, and batch-scan recipes
macs/                  Geant4 and simulation configuration macros
src/                   simulation executable and cosine shared library
test/                  Catch2 tests and test-runner configuration
```

## License

cosine is distributed under the GNU General Public License, version 3. See [LICENSE](LICENSE).
