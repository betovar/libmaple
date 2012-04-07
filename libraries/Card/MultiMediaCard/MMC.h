typedef enum UHSOperationMode {
  // Ultra High Speed Phase I (UHS-I) Card Modes 3.3v
  UHS_OM_DS = 0,    // Default Speed
  UHS_OM_HS,        // High Speed
  // The folliwing bus speeds require 1.8v signaling
  UHS_OM_SDR12,
  UHS_OM_SDR25,
  UHS_OM_SDR50,
  UHS_OM_SDR104,
  UHS_OM_DDR50,
} UHSOperationMode;
