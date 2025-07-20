<div align="center"><img src="assets/dynamixel.png"></div>
<br>
<h1 align="center">Dynamixel Library for STM32</h1>

<div align="center">

![license](https://img.shields.io/github/license/leoraclet/dynamixel)
![language](https://img.shields.io/github/languages/top/leoraclet/dynamixel)
![lastcommit](https://img.shields.io/github/last-commit/leoraclet/dynamixel) <br>
![Language](https://img.shields.io/badge/Language-C-1d50de)
![Libraries](https://img.shields.io/badge/Framework-STM32-fa8925)
![Size](https://img.shields.io/badge/Size-6.7Mo-f12222) ![Open
Source](https://badges.frapsoft.com/os/v2/open-source.svg?v=103)


</div>

## Table of Contents
- [Table of Contents](#table-of-contents)
- [🙏 Credits](#-credits)
- [📖 About](#-about)
- [📦 Structure](#-structure)
- [🔧 Build](#-build)
- [📜 License](#-license)

## 🙏 Credits

- Thanks to [FluffLescure](https://github.com/FluffLescure) for his contribution, naming
  - Improving the code
  - Fixing some bugs on reading functions


## 📖 About

This a simple STM32 project that includes a library to control and interact with the
[**AX-12**](https://emanual.robotis.com/docs/en/dxl/ax/ax-12a/) DYNAMIXEL.

> [!NOTE]
>
> This project was setup with
> [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) with the
> [STM32F411CEU6](https://www.st.com/en/microcontrollers-microprocessors/stm32f411ce.html) as the
> specified target MCU

## 📦 Structure

**Directories**

  - [**`Src`**](./Core/Src/) - Source files (`.c`)
  - [**`Inc`**](./Core/Inc/) - Headers (`.h`)

**Files**

  - `dynamixel.c` - Module to interact with the AX-12


## 🔧 Build

> [!WARNING]
>
> STM32CubeIDE builds the project only for the specified target (MCU)

To build this project, first install the
[STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) on your system.

Then, you can clone this repo

```bash
git clone https://github.com/leoraclet/dynamixel
```

And open / import the project into STM32CubeIDE

---

If you want to build for another target, then create a new project for the wanted target and just
copy the source files of this project, with the corresponding headers, into your project.

## 📜 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.