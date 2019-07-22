import React, { Component } from 'react';
import {
  StyleSheet,
  Text,
  View,
  TouchableOpacity,
  FlatList,
  TextInput,
  Platform,
  Alert,
  TouchableHighlight
} from 'react-native';
import { Button } from 'react-native-paper';
import { Buffer } from 'buffer';

import BleModule from './BleModule.js';
global.BluetoothManager = new BleModule();

export default class App extends Component {
  constructor(props) {
    super(props);
    this.state = {
      scanning: false,
      isConnected: false,
      text: '',
      writeData: '',
      receiveData: '',
      readData: '',
      data: [],
      isMonitoring: false
    };
    this.bluetoothReceiveData = [];
    this.deviceMap = new Map();
    setTimeout(() => {
      this.scan();
    }, 100);
  }

  componentWillUnmount() {
    this.onStateChangeListener = BluetoothManager.manager.onStateChange(state => {
      console.log('onStateChange: ', state);
      if (state == 'PoweredOn') {
        this.scan();
      }
    });
    BluetoothManager.destroy();
    this.onStateChangeListener && this.onStateChangeListener.remove();
    this.disconnectListener && this.disconnectListener.remove();
    this.monitorListener && this.monitorListener.remove();
  }

  alert(text) {
    Alert.alert('Alert', text, [{ text: 'OK', onPress: () => {} }]);
  }

  scan() {
    if (!this.state.scanning) {
      this.setState({ scanning: true });
      this.deviceMap.clear();
      BluetoothManager.manager.startDeviceScan(null, null, (error, device) => {
        if (error) {
          alert('startDeviceScan error:' + JSON.stringify(error));
          if (error.errorCode == 102) {
            this.alert('turn on bluetooth');
          }
          this.setState({ scanning: false });
        } else {
          console.log(device.id, device.name);
          if (device.name && device.name.startsWith('waive')) {
            this.deviceMap.set(device.id, device);
            this.setState({ data: [...this.deviceMap.values()] });
          }
        }
      });
      this.scanTimer && clearTimeout(this.scanTimer);
      this.scanTimer = setTimeout(() => {
        if (this.state.scanning) {
          BluetoothManager.stopScan();
          this.setState({ scanning: false });
        }
      }, 1000);
    } else {
      BluetoothManager.stopScan();
      this.setState({ scanning: false });
    }
  }

  connect(item) {
    if (this.state.scanning) {
      BluetoothManager.stopScan();
      this.setState({ scanning: false });
    }
    if (BluetoothManager.isConnecting) {
      console.log('connecting, cannot connect another one');
      return;
    }
    let newData = [...this.deviceMap.values()];
    newData[item.index].isConnecting = true;
    this.setState({ data: newData });
    BluetoothManager.connect(item.item.id)
      .then(device => {
        newData[item.index].isConnecting = false;
        this.setState({ data: [newData[item.index]], isConnected: true });
        this.onDisconnect();
      })
      .catch(err => {
        newData[item.index].isConnecting = false;
        this.setState({ data: [...newData] });
        this.alert(err);
      });
  }

  read = index => {
    BluetoothManager.read(index)
      .then(value => {
        this.setState({ readData: value });
      })
      .catch(err => {});
  };

  write = (index, type) => {
    if (this.state.text.length == 0) {
      this.alert('enter text');
      return;
    }
    BluetoothManager.write(this.state.text, index, type)
      .then(characteristic => {
        this.bluetoothReceiveData = [];
        this.setState({
          writeData: this.state.text,
          text: ''
        });
      })
      .catch(err => {});
  };

  writeWithoutResponse = (index, type) => {
    if (this.state.text.length == 0) {
      this.alert('enter text');
      return;
    }
    BluetoothManager.writeWithoutResponse(this.state.text, index, type)
      .then(characteristic => {
        this.bluetoothReceiveData = [];
        this.setState({
          writeData: this.state.text,
          text: ''
        });
      })
      .catch(err => {});
  };

  monitor = index => {
    let transactionId = 'monitor';
    this.monitorListener = BluetoothManager.manager.monitorCharacteristicForDevice(
      BluetoothManager.peripheralId,
      BluetoothManager.nofityServiceUUID[index],
      BluetoothManager.nofityCharacteristicUUID[index],
      (error, characteristic) => {
        if (error) {
          this.setState({ isMonitoring: false });
          console.log('monitor fail:', error);
          this.alert('monitor fail: ' + error.reason);
        } else {
          let value = new Buffer(characteristic.value, 'base64').toString('ascii');
          this.setState({ isMonitoring: true });
          this.bluetoothReceiveData.push(value);
          this.setState({ receiveData: this.bluetoothReceiveData.join('') });
          console.log('monitor success', value);
        }
      },
      transactionId
    );
  };

  onDisconnect() {
    this.disconnectListener = BluetoothManager.manager.onDeviceDisconnected(
      BluetoothManager.peripheralId,
      (error, device) => {
        if (error) {
          console.log('onDeviceDisconnected', 'device disconnect', error);
          this.setState({
            data: [...this.deviceMap.values()],
            isConnected: false
          });
        } else {
          this.disconnectListener && this.disconnectListener.remove();
          console.log('onDeviceDisconnected', 'device disconnect', device.id, device.name);
        }
      }
    );
  }

  disconnect() {
    BluetoothManager.disconnect()
      .then(res => {
        this.setState({
          data: [...this.deviceMap.values()],
          isConnected: false
        });
      })
      .catch(err => {
        this.setState({
          data: [...this.deviceMap.values()],
          isConnected: false
        });
      });
  }

  renderItem = item => {
    let data = item.item;
    return (
      <TouchableOpacity
        activeOpacity={0.7}
        disabled={this.state.isConnected ? true : false}
        onPress={() => {
          this.connect(item);
        }}
        style={styles.item}
      >
        <View style={{ flexDirection: 'row' }}>
          <Text style={{ color: 'black' }}>{data.name ? data.name : ''}</Text>
          <Text style={{ color: 'red', marginLeft: 50 }}>
            {data && data.isConnecting ? 'connecting...' : ''}
          </Text>
        </View>
        <Text>{data.id}</Text>
      </TouchableOpacity>
    );
  };

  renderHeader = () => {
    return (
      <View style={{ marginTop: 20 }}>
        <TouchableOpacity
          activeOpacity={0.7}
          style={[styles.buttonView, { marginHorizontal: 10, height: 40, alignItems: 'center' }]}
          onPress={this.state.isConnected ? this.disconnect.bind(this) : this.scan.bind(this)}
        >
          <Text style={styles.buttonText}>
            {this.state.scanning
              ? 'Scanning'
              : this.state.isConnected
              ? 'Disconnect'
              : 'Scan waive*'}
          </Text>
        </TouchableOpacity>
      </View>
    );
  };

  renderFooter = () => {
    return (
      <View style={{ marginBottom: 30 }}>
        {this.state.isConnected ? (
          <View>
            {this.renderWriteView(
              'write：',
              'send',
              BluetoothManager.writeWithResponseCharacteristicUUID,
              this.write
            )}
            {this.renderWriteView(
              'writeWithoutResponse：',
              'send',
              BluetoothManager.writeWithoutResponseCharacteristicUUID,
              this.writeWithoutResponse
            )}
            {this.renderReceiveView(
              'read：',
              'read',
              BluetoothManager.readCharacteristicUUID,
              this.read,
              this.state.readData
            )}
            {this.renderReceiveView(
              `monitored data：${this.state.isMonitoring ? 'monitoring' : 'not monitoring'}`,
              'start monitoring',
              BluetoothManager.nofityCharacteristicUUID,
              this.monitor,
              this.state.receiveData
            )}
          </View>
        ) : (
          <View style={{ marginBottom: 20 }} />
        )}
      </View>
    );
  };

  renderWriteView = (label, buttonText, characteristics, onPress, state) => {
    if (characteristics.length == 0) {
      return null;
    }
    return (
      <View style={{ marginHorizontal: 10, marginTop: 30 }} behavior="padding">
        <Text style={{ color: 'black' }}>{label}</Text>
        <Text style={styles.content}>{this.state.writeData}</Text>
        {characteristics.map((item, index) => {
          return (
            <TouchableOpacity
              key={index}
              activeOpacity={0.7}
              style={styles.buttonView}
              onPress={() => {
                onPress(index);
              }}
            >
              <Text style={styles.buttonText}>
                {buttonText} ({item})
              </Text>
            </TouchableOpacity>
          );
        })}
        <TextInput
          style={[styles.textInput]}
          value={this.state.text}
          placeholder="enter text"
          onChangeText={text => {
            this.setState({ text: text });
          }}
        />
        <View style={styles.row}>
          <View style={styles.columnGap} />
          <TouchableHighlight
            style={styles.buttonTouchable}
            onPress={() => {
              this.setState({ text: '{"lock":"open"}' });
              setTimeout(() => {
                onPress(0);
              }, 10);
            }}
          >
            <Button style={styles.button} mode="contained">
              lock: open
            </Button>
          </TouchableHighlight>
          <View style={styles.columnGap} />
          <TouchableHighlight
            style={styles.buttonTouchable}
            onPress={() => {
              this.setState({ text: '{"lock":"close"}' });
              setTimeout(() => {
                onPress(0);
              }, 10);
            }}
          >
            <Button style={styles.button} mode="contained">
              lock: close
            </Button>
          </TouchableHighlight>
          <View style={styles.columnGap} />
        </View>
        <View style={styles.row}>
          <View style={styles.columnGap} />
          <TouchableHighlight
            style={styles.buttonTouchable}
            onPress={() => {
              this.setState({ text: '{"immo":"lock"}' });
              setTimeout(() => {
                onPress(0);
              }, 10);
            }}
          >
            <Button style={styles.button} mode="contained">
              immo: lock
            </Button>
          </TouchableHighlight>
          <View style={styles.columnGap} />
          <TouchableHighlight
            style={styles.button}
            onPress={() => {
              this.setState({ text: '{"immo":"unlock"}' });
              setTimeout(() => {
                onPress(0);
              }, 10);
            }}
          >
            <Button style={styles.button} mode="contained">
              immo: unlock
            </Button>
          </TouchableHighlight>
          <View style={styles.columnGap} />
        </View>
        <View style={styles.row}>
          <View style={styles.columnGap} />
          <TouchableHighlight
            style={styles.buttonTouchable}
            onPress={() => {
              this.setState({ text: '{"inRide":"true"}}' });
              setTimeout(() => {
                onPress(0);
              }, 10);
            }}
          >
            <Button style={styles.button} mode="contained">
              inRide: true
            </Button>
          </TouchableHighlight>
          <View style={styles.columnGap} />
          <TouchableHighlight
            style={styles.button}
            onPress={() => {
              this.setState({ text: '{"inRide":"false"}' });
              setTimeout(() => {
                onPress(0);
              }, 10);
            }}
          >
            <Button style={styles.button} mode="contained">
              inRide: false
            </Button>
          </TouchableHighlight>
          <View style={styles.columnGap} />
        </View>
        <View style={styles.row}>
          <View style={styles.columnGap} />
          <TouchableHighlight
            style={styles.buttonTouchable}
            onPress={() => {
              this.setState({ text: '{"reboot":"true"}}' });
              setTimeout(() => {
                onPress(0);
              }, 10);
            }}
          >
            <Button style={styles.button} mode="contained">
              reboot: true
            </Button>
          </TouchableHighlight>
          <View style={styles.columnGap} />
          <TouchableHighlight
            style={styles.button}
            onPress={() => {
              this.setState({ text: '{"?":"?"}' });
              setTimeout(() => {
                onPress(0);
              }, 10);
            }}
          >
            <Button style={styles.button} mode="contained">
              ?: ?
            </Button>
          </TouchableHighlight>
          <View style={styles.columnGap} />
        </View>
      </View>
    );
  };

  renderReceiveView = (label, buttonText, characteristics, onPress, state) => {
    if (characteristics.length == 0) {
      return null;
    }
    return (
      <View style={{ marginHorizontal: 10, marginTop: 30 }}>
        <Text style={{ color: 'black', marginTop: 5 }}>{label}</Text>
        <Text style={styles.content}>{state}</Text>
        {characteristics.map((item, index) => {
          return (
            <TouchableOpacity
              activeOpacity={0.7}
              style={styles.buttonView}
              onPress={() => {
                onPress(index);
              }}
              key={index}
            >
              <Text style={styles.buttonText}>
                {buttonText} ({item})
              </Text>
            </TouchableOpacity>
          );
        })}
      </View>
    );
  };

  render() {
    return (
      <View style={styles.container}>
        <FlatList
          renderItem={this.renderItem}
          keyExtractor={item => item.id}
          data={this.state.data}
          ListHeaderComponent={this.renderHeader}
          ListFooterComponent={this.renderFooter}
          extraData={[
            this.state.isConnected,
            this.state.text,
            this.state.receiveData,
            this.state.readData,
            this.state.writeData,
            this.state.isMonitoring,
            this.state.scanning
          ]}
          keyboardShouldPersistTaps="handled"
        />
      </View>
    );
  }
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: 'white',
    marginTop: Platform.OS == 'ios' ? 20 : 0
  },
  item: {
    flexDirection: 'column',
    borderColor: 'rgb(235,235,235)',
    borderStyle: 'solid',
    borderBottomWidth: StyleSheet.hairlineWidth,
    paddingLeft: 10,
    paddingVertical: 8
  },
  buttonView: {
    height: 30,
    backgroundColor: 'rgb(33, 150, 243)',
    paddingHorizontal: 10,
    borderRadius: 5,
    justifyContent: 'center',
    alignItems: 'center',
    alignItems: 'flex-start',
    marginTop: 10
  },
  buttonText: {
    color: 'white',
    fontSize: 12
  },
  content: {
    marginTop: 5,
    marginBottom: 15
  },
  textInput: {
    paddingLeft: 5,
    paddingRight: 5,
    backgroundColor: 'white',
    height: 50,
    fontSize: 16,
    flex: 1
  },
  row: { flexDirection: 'row', height: 70, marginBottom: 10 },
  columnGap: { width: 10 },
  buttonTouchable: { flex: 1, borderRadius: 4 },
  button: {
    flex: 1,
    justifyContent: 'center',
    backgroundColor: 'rgb(33, 150, 243)'
  }
});
