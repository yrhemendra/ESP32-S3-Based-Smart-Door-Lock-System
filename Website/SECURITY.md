# Security Features

This app now includes comprehensive security features:

## Features Added

### PIN Management
- **Set PIN**: Create a 4-digit PIN for app access
- **Change PIN**: Update your PIN after verifying the current one
- **PIN Verification**: Secure PIN storage using Expo SecureStore

### Biometric Authentication
- **Fingerprint/Face ID**: Use device biometrics when available
- **Fallback to PIN**: Automatically falls back to PIN if biometric fails
- **Device Compatibility**: Checks for biometric hardware and enrollment

### Security Flow
1. **First Launch**: App allows access without authentication
2. **Set PIN**: Navigate to Security tab to set up PIN protection
3. **Authentication**: Once PIN is set, app requires authentication on launch
4. **Biometric Setup**: Enable fingerprint/Face ID for faster access

## Usage

1. **Install dependencies**:
   ```bash
   npm install
   ```

2. **Start the app**:
   ```bash
   npx expo start
   ```

3. **Set up security**:
   - Open the app
   - Go to "Security" tab
   - Set up your PIN
   - Enable biometric authentication (if available)

## Security Components

- `SecurityContext`: Manages authentication state
- `PinInput`: Custom PIN input component
- `AuthScreen`: Authentication screen for app entry
- `SecurityScreen`: Settings for PIN and biometric management

## Dependencies

- `expo-local-authentication`: Biometric authentication
- `expo-secure-store`: Secure PIN storage