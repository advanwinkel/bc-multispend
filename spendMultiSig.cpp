#include <bitcoin/bitcoin.hpp>
#include "HD_Wallet.cpp"
#include <string.h>

using namespace bc;
using namespace bc::wallet;
using namespace bc::machine;
using namespace bc::chain;

script getRedeemScript(data_chunk pubkey1, data_chunk pubkey2, data_chunk pubkey3)
{
	operation::list opList {operation(opcode(82)), operation(pubkey1), operation(pubkey2), operation(pubkey3), operation(opcode(83)), operation(opcode(174))};
	script multisigScript(opList);
	return multisigScript;
}

output_point getUTXO()
{
	//std::string txhash;/// need this
	hash_digest prevOutHash = hash_literal("5148e6b786b048aecad54ea25957797c5bf9d0896ce21d7fe5a48987f92d49ff");
	return output_point(prevOutHash, 0u);
}

script makeP2KH(short_hash hash)
{
	return script(script().to_pay_key_hash_pattern(hash));

}

script makeP2SH(short_hash hash)
{
	return script(script().to_pay_script_hash_pattern(hash));

}

 input makeInput()
 {
 	input input1 = input();
 	input1.set_previous_output(getUTXO());
 	input1.set_sequence(0xffffffff);
 	//short_hash scriptHash = bitcoin_short_hash(.to_data(0));
 	//script outredeemScript = getRedeemScript(p1, p2, p3);
 	//input1.set_script(outScript);
 	return input1;
 }

uint64_t getAmount()
{
	std::string input = "55000";
	uint64_t amount;
	decode_base10(amount, input);
	return amount;
}

script getOutputScript()
{
	std::string destinationAddress = "n2Uuf8rA64w3nW5aga5uMfHLjxRpT56yco";
	payment_address sender(destinationAddress);
	if(sender.hash()[-1] == 0x6F)
	{
		return makeP2KH(sender.hash());

	} else if (sender.hash()[-1] == 0xC4){
		return makeP2SH(sender.hash());
	} else {
		std::cout << "invalid address destination" << std::endl;
		return makeP2KH(sender.hash());
	}

}

output makeOutput()
{
	uint64_t amount = getAmount();
	script lockingScript = getOutputScript();
	return output(amount, lockingScript);
}

transaction makeTX()
{
	input::list inpList;
	output::list outpList;
	inpList.push_back(makeInput());
	outpList .push_back(makeOutput());
	transaction tx = transaction(1, 0, inpList, outpList);
	return tx;
}

endorsement signTX(HD_Wallet wallet, script multi, transaction tx)
{
	endorsement endorsed;
	script().create_endorsement(endorsed, wallet.childPrivateKey(1).secret(), multi, tx, 0, all);
	return endorsed;
}
endorsement makeSigScript(HD_Wallet wallet, endorsement sig, script redeem, transaction tx, data_chunk pubkey1, data_chunk pubkey2, data_chunk pubkey3)
{
	
	endorsement endorsed;
	
	operation::list ops {operation(sig), operation(opcode(82)), operation(pubkey1), operation(pubkey2), operation(pubkey3), operation(opcode(83)), operation(opcode(175))}; //, operation(opcode(82)), operation(pubkey1), operation(pubkey2), operation(pubkey3), operation(opcode(83)), operation(opcode(175))};;
	script newSig(ops);
	script().create_endorsement(endorsed, wallet.childPrivateKey(1).secret(), newSig, tx, 0, all);


	return endorsed;

}
script makeSigScript2(endorsement endorsement1, endorsement endorsement2, script multi)
{
	data_chunk opData = multi.to_data(0);
	operation::list ops {operation(opcode(0)), operation(endorsement1), operation(endorsement2), operation(opData)};
	
	return script(ops);
}

int main()
{
	std::string Mnemonic1 = "cat unable welcome inflict unable save endorse travel ask space ski account";
	std::string Mnemonic2 = "quality grocery tornado invest surround smile hire drama oppose february candy clog";
	std::string Mnemonic3 = "job cupboard soon pilot coconut truth wild affair smooth seminar feel spell";

	HD_Wallet wallet1(split(Mnemonic1));
	HD_Wallet wallet2(split(Mnemonic2));
	HD_Wallet wallet3(split(Mnemonic3));

	data_chunk pubkey1 = to_chunk(wallet1.childPublicKey(1).point());
	data_chunk pubkey2 = to_chunk(wallet2.childPublicKey(1).point());
	data_chunk pubkey3 = to_chunk(wallet3.childPublicKey(1).point());

	script multi = getRedeemScript(pubkey1, pubkey2, pubkey3);

	std::cout << "\nRedeem Script" << std::endl;
	std::cout << multi.to_string(0xffffffff) << std::endl;

	std::cout << "Payment Address" << std::endl;
	std::cout << payment_address(multi, 0xc4).encoded() << std::endl;

	transaction tx = makeTX();
	std::cout << "\n" <<encode_base16(tx.to_data()) << std::endl;

	std::cout << "\nendorsement 1: " << std::endl;
	endorsement endorsed1 = signTX(wallet1, multi, tx);
	// std::cout << encode_base16(endorsed1) << std::endl;
	endorsement endorsed2 = signTX(wallet2, multi, tx);
	script finalSigScript = makeSigScript2(endorsed1, endorsed2, multi);
	std::cout <<"\nSignature Script: " << std::endl;
	std::cout << finalSigScript.to_string(0xffffffff) << std::endl;

	tx.inputs()[0].set_script(finalSigScript);

	std::cout << tx.total_input_value() << std::endl;
	std::cout << tx.total_output_value() << std::endl;
	std::cout << tx.is_valid() << std::endl; 
	std::cout << tx.signature_operations(1) << std::endl;
	std::cout << encode_base16(tx.to_data()) << std::endl;
	data_stack pubStack {pubkey1, pubkey2, pubkey3};
	std::cout << "\nTEST: " << std::endl;
	std::cout << script(script().to_pay_multisig_pattern(2, pubStack)).to_string(1) << std::endl;
	//data_chunk testerScript;
	//decode_base16(testerScript, );
	//std::cout << redeemScript.to_string(0xffffffff) << "\n" << std::endl;
	std::cout << finalSigScript.to_string(0xffffffff) << std::endl;
}