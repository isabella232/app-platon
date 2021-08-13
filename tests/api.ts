import SpeculosTransport from "@ledgerhq/hw-transport-node-speculos";
import AppLat from "@ledgerhq/hw-app-eth";

function getLat() : Promise<AppLat> {
    const apduPort = 9998;
    return  SpeculosTransport.open({ apduPort }).then((transport) => { return new AppLat(transport);});
}

const getLatAddress = async (path: string) => {
    const lat =  await getLat();
    const result = await lat.getAddress(path, false, false);
    return result;
};

const signLatTransaction = async (path: string, rawTxHex: string) => {
    const lat =  await getLat();
    const result = await lat.signTransaction(path, rawTxHex);
    return result;
};

const signLatPersonalMessage = async (path: string, messageHex: string) => {
    const lat =  await getLat();
    const result = await lat.signPersonalMessage(path, messageHex);
    return result;
};

export {getLatAddress, signLatTransaction, signLatPersonalMessage};