# Smart Lock Mobile App

A React Native Expo app for controlling and managing a smart lock system with PIN and fingerprint authentication.

## Features

### 🔐 Security Management
- **PIN Setup & Change**: Set and modify 4-digit PIN codes
- **Fingerprint Enrollment**: Enroll fingerprints with usernames via smart lock sensor
- **Biometric Authentication**: Use device biometrics when available
- **Secure Storage**: PIN data stored securely using Expo SecureStore

### 🏠 Smart Lock Control
- **Lock/Unlock**: Control smart lock remotely
- **PIN Verification**: Require PIN verification before unlock commands
- **Real-time Status**: View current lock status and connectivity
- **ESP32 Integration**: Direct communication with ESP32 smart lock device

### 📊 Access History
- **Activity Log**: View all lock/unlock activities
- **User Tracking**: See which user performed each action
- **Authentication Method**: Track whether PIN, fingerprint, or manual access was used
- **Timestamps**: Detailed time information for each event

## Project Structure

```
app/
├── (tabs)/
│   ├── index.tsx          # Home screen with lock controls
│   ├── security.tsx       # Security settings tab
│   └── _layout.tsx        # Tab navigation layout
├── history.tsx            # Access history screen
├── security.tsx           # Security settings (standalone)
└── _layout.tsx            # Root app layout

components/
├── ui/
│   ├── icon-symbol.tsx    # Icon component
│   └── icon-symbol.ios.tsx
├── AuthScreen.tsx         # Authentication screen
├── PinInput.tsx           # Custom PIN input keypad
├── PinVerificationModal.tsx # PIN verification modal
├── UsernameInputModal.tsx # Username input for fingerprint enrollment
└── haptic-tab.tsx         # Haptic feedback for tabs

contexts/
└── SecurityContext.tsx    # Security state management

services/
└── SmartHubService.ts     # API service for smart hub communication

config/
└── SmartHubConfig.ts      # Smart hub configuration
```

## Installation

1. **Install dependencies**:
   ```bash
   npm install
   ```

2. **Start the development server**:
   ```bash
   npx expo start
   ```

3. **Configure Smart Hub**:
   - Update `config/SmartHubConfig.ts` with your smart hub IP address
   - Ensure ESP32 device ID matches (`lock_01`)

## Usage

### First Time Setup
1. Open the app
2. Go to Security tab
3. Set up your PIN
4. Enroll fingerprints (requires username)

### Daily Use
1. **Unlock**: Tap unlock button → Enter PIN → Door unlocks
2. **Lock**: Tap lock button (no PIN required)
3. **View History**: Check access logs with user details

## API Integration

The app communicates with your ESP32 smart lock via the smart hub API:

- **Lock Commands**: `POST /api/device/command`
- **Device Status**: `GET /api/device/status`
- **Access History**: `GET /api/device/history`
- **Fingerprint Enrollment**: `POST /api/device/command` (enroll action)

## Dependencies

- **expo-local-authentication**: Device biometric authentication
- **expo-secure-store**: Secure PIN storage
- **expo-router**: File-based navigation
- **react-native**: Core framework

## Security Features

- PIN verification required for unlock commands
- Secure local PIN storage
- Biometric fallback authentication
- Real-time device communication
- User activity tracking