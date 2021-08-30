import {
    getLatAddress, getLatAppConfiguration, signLatTransaction, 
    signLatPersonalMessage} from "./api"

getLatAddress("44'/486'/0'/0/0").then(a => console.log(a));
getLatAppConfiguration().then(a => console.log(a));
signLatTransaction("44'/486'/0'/0'/0", "e8018504e3b292008252089428ee52a8f3d6e5d15f8b131996950d7f296c7952872bd72a2487400080").then(a => console.log(a));
signLatPersonalMessage("44'/486'/0'/0'/0", Buffer.from("test").toString("hex")).then(a => console.log(a));
