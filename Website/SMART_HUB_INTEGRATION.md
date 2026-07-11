# Smart Hub Integration Setup

## Overview
This app integrates with your smart lock hub for fingerprint enrollment and lock control. PIN management is handled locally by the app, while fingerprint operations communicate with the smart lock device.

## Configuration

### 1. Smart Hub Connection
Update the configuration in `config/SmartHubConfig.ts`:

```typescript
export const SMART_HUB_CONFIG = {
  baseUrl: 'http://YOUR_HUB_IP:PORT', // Replace with your hub's IP
  // ... other settings
};
```

### 2. API Endpoints
The app expects these endpoints on your smart hub:

#### Fingerprint Management
- `POST /api/fingerprint/enroll` - Start fingerprint enrollment
- `GET /api/fingerprint/status/{enrollmentId}` - Check enrollment status
- `GET /api/fingerprint/list/{userId}` - Get enrolled fingerprints
- `DELETE /api/fingerprint/delete/{fingerprintId}` - Delete fingerprint

#### Lock Control
- `POST /api/lock/command` - Send lock/unlock commands
- `GET /api/device/status` - Get device status

## Features

### PIN Management (App-based)
- Set 4-digit PIN locally
- Change PIN with current PIN verification
- Secure storage using Expo SecureStore

### Fingerprint Management (Smart Lock-based)
- Enroll fingerprints via smart lock sensor
- View enrolled fingerprints list
- Delete fingerprints from smart lock
- Real-time enrollment status

### Lock Control
- Lock/unlock via app
- Support for PIN and fingerprint authentication
- Device status monitoring

## Usage Flow

1. **Initial Setup**:
   - Configure smart hub IP in config file
   - Set PIN in Security tab
   - Enroll fingerprints using smart lock sensor

2. **Fingerprint Enrollment**:
   - Tap "Enroll New Fingerprint" in Security tab
   - Follow prompts to use smart lock sensor
   - Fingerprint is stored on smart lock device

3. **Lock Control**:
   - Use home screen to lock/unlock
   - Authentication via PIN (app) or fingerprint (smart lock)

## API Request/Response Format

### Fingerprint Enrollment Request
```json
{
  "userId": "mobile_app_user"
}
```

### Fingerprint Enrollment Response
```json
{
  "success": true,
  "message": "Enrollment started",
  "data": {
    "enrollmentId": "enroll_123",
    "status": "pending"
  }
}
```

### Lock Command Request
```json
{
  "command": "lock",
  "authMethod": "pin",
  "authData": "1234"
}
```

### Device Status Response
```json
{
  "success": true,
  "data": {
    "locked": true,
    "connected": true,
    "batteryLevel": 85,
    "lastActivity": "2024-01-01T12:00:00Z"
  }
}
```

## Error Handling
- Network timeouts (10 second default)
- Connection failures with fallback messages
- Invalid responses with user-friendly alerts