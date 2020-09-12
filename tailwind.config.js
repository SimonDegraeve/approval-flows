module.exports = {
  theme: {},
  purge: {
    content: ['./src/**/*.html', './src/**/*.re'],
  },
  future: {
    removeDeprecatedGapUtilities: true,
    purgeLayersByDefault: true,
  },
}
