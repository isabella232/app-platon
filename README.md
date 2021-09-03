# app-lat

Platon wallet application framework for Nano S and Nano X.

This app follows the specification available in the `doc/` folder.

# Plugins

This app support external plugins. More info in [doc/latapp_plugin.md](./doc/latapp_plugins.md). If you wish to have a look at an existing plugin, feel free to check out the [ParaSwap plugin](https://github.com/LedgerHQ/app-plugin-paraswap).

# Testing

## Start the speculos emulator

```bash
./speculos.py bin/app.elf
```

> Speculos information reference: https://github.com/LedgerHQ/speculos.

## Running tests

- Install dependencies

  First [install yarn](https://classic.yarnpkg.com/en/docs/install/#debian-stable) or [install node.js](https://nodejs.org/en/).

  ```bash
  npm install
  ```

- Install TypeScript

  ```bash
  npm install -g typescript
  ```

- Generate js file

  ```bash
  tsc test.ts
  ```

  > The installed version can be checked by the `tsc -v` command.

- Perform unit tests

  ```bash
  npm run test
  ```

## Adding tests

Add a test interface to the test.ts file, and then regenerate the js file to continue running the unit test.