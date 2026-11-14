/**
 * Cryptographic Wallet & Digital Signature Utility
 * Built with Node.js native crypto module (ECDSA prime256v1 / SHA-256)
 */

const crypto = require('node:crypto');

class Wallet {
  /**
   * Generates a new cryptographic keypair for a voter
   * @param {string} [voterNationalId] Optional voter ID or alias to bind
   * @returns {Object} { voterId, voterAddress, publicKey, privateKey, registrationHash }
   */
  static generateVoterCredentials(voterNationalId = null) {
    const { publicKey, privateKey } = crypto.generateKeyPairSync('ec', {
      namedCurve: 'prime256v1',
      publicKeyEncoding: {
        type: 'spki',
        format: 'pem'
      },
      privateKeyEncoding: {
        type: 'pkcs8',
        format: 'pem'
      }
    });

    const voterId = voterNationalId || `VTR-${crypto.randomBytes(4).toString('hex').toUpperCase()}`;
    const voterAddress = Wallet.deriveVoterAddress(publicKey);
    const registrationHash = crypto
      .createHash('sha256')
      .update(`${voterId}:${voterAddress}`)
      .digest('hex');

    return {
      voterId,
      voterAddress,
      publicKey,
      privateKey,
      registrationHash,
      createdAt: new Date().toISOString()
    };
  }

  /**
   * Derives a deterministic public address from the public key
   * @param {string} publicKeyPem 
   * @returns {string} Hex address prefixed with 0x
   */
  static deriveVoterAddress(publicKeyPem) {
    const cleanKey = publicKeyPem.replace(/-----[\w\s]+-----/g, '').replace(/\s+/g, '');
    const hash = crypto.createHash('sha256').update(cleanKey).digest('hex');
    return `0x${hash.slice(0, 40)}`;
  }

  /**
   * Digitally signs payload data using voter's private key
   * @param {string} privateKeyPem 
   * @param {string|Object} payload 
   * @returns {string} Hex encoded signature
   */
  static signData(privateKeyPem, payload) {
    const serialized = typeof payload === 'string' ? payload : JSON.stringify(payload);
    const sign = crypto.createSign('SHA256');
    sign.update(serialized);
    sign.end();
    return sign.sign(privateKeyPem, 'hex');
  }

  /**
   * Verifies digital signature against the voter's public key
   * @param {string} publicKeyPem 
   * @param {string|Object} payload 
   * @param {string} signatureHex 
   * @returns {boolean} True if signature is cryptographically valid
   */
  static verifySignature(publicKeyPem, payload, signatureHex) {
    try {
      const serialized = typeof payload === 'string' ? payload : JSON.stringify(payload);
      const verify = crypto.createVerify('SHA256');
      verify.update(serialized);
      verify.end();
      return verify.verify(publicKeyPem, signatureHex, 'hex');
    } catch (err) {
      return false;
    }
  }

  /**
   * Hashes any data string using SHA-256
   * @param {string} data 
   * @returns {string} Hex string
   */
  static sha256(data) {
    return crypto.createHash('sha256').update(data).digest('hex');
  }
}

module.exports = Wallet;
