'use strict';
const path = require('path');

module.exports = function () {
  return {
    target: 'node',
    output: {
      path: path.join(__dirname, '/dist/'),
      library: 'truffle-test-helpers',
      libraryTarget: 'commonjs2',
      filename: 'index.js',
    },
    entry: [
      path.join(__dirname, 'src/index.js')
    ],
    resolve: {
      extensions: ['.js'],
      modules: ['node_modules']
    },
    module: {
      rules: [
        {
          test: /\.js?$/,
          exclude: /node_modules/,
          use: [
            'babel-loader',
            'eslint-loader'
          ]
        }
      ]
    }
  };
};
