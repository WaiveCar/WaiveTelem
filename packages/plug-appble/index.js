/**
 * @format
 */

import { AppRegistry, PermissionsAndroid, Platform, YellowBox } from 'react-native';
import App from './App';
import { name as appName } from './app.json';

YellowBox.ignoreWarnings(['Remote debugger']);

async function requestLocationPermission() {
  try {
    const granted = await PermissionsAndroid.request(
      PermissionsAndroid.PERMISSIONS.ACCESS_COARSE_LOCATION,
      {
        title: 'App Location Permission',
        message: 'APPBLE App needs access to your location ' + 'so you can use bluetooth.',
        buttonNeutral: 'Ask Me Later',
        buttonNegative: 'Cancel',
        buttonPositive: 'OK'
      }
    );
    if (granted === PermissionsAndroid.RESULTS.GRANTED) {
      console.log('permission granted');
    } else {
      console.log('permission denied');
    }
  } catch (err) {
    console.warn(err);
  }
}

if (Platform.OS === 'android') {
  requestLocationPermission();
}

AppRegistry.registerComponent(appName, () => App);
