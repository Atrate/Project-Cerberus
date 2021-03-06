"""
Copyright (c) Microsoft Corporation. All rights reserved.
Licensed under the MIT license.
"""

from __future__ import print_function
from __future__ import unicode_literals
import re
import binascii
import traceback
import xml.etree.ElementTree as et
import manifest_types


XML_ID_ATTRIB = "id"
XML_VERSION_ATTRIB = "version"
XML_PLATFORM_ATTRIB = "platform"
XML_LEVEL_ATTRIB = "level"

XML_FW_TAG = "Firmware"
XML_DIGEST_TAG = "Digest"
XML_FAILURE_ACTION_TAG = "FailureAction"
XML_RW_TAG = "ReadWrite"
XML_REGION_TAG = "Region"
XML_IMG_TAG = "SignedImage"
XML_START_ADDR_TAG = "StartAddr"
XML_END_ADDR_TAG = "EndAddr"
XML_PB_KEY_TAG = "PublicKey"
XML_SIG_TAG = "Signature"
XML_VALIDATE_TAG = "ValidateOnBoot"
XML_VERSION_ADDR_TAG = "VersionAddr"
XML_UNUSED_BYTE_TAG = "UnusedByte"
XML_VERSION_TAG = "Version"
XML_ROT_TAG = "RoT"
XML_PORTS_TAG = "Ports"
XML_PORT_TAG = "Port"
XML_INTERFACE_TAG = "Interface"
XML_ADDRESS_TAG = "Address"
XML_BMC_ADDRESS_TAG = "BMCAddress"
XML_EID_TAG = "EID"
XML_CPLD_TAG = "CPLD"
XML_CHANNEL_TAG = "Channel"
XML_COMPONENTS_TAG = "Components"
XML_COMPONENT_TAG = "Component"
XML_DEVICETYPE_TAG = "DeviceType"
XML_BUS_TAG = "Bus"
XML_I2CMODE_TAG = "I2CMode"
XML_PWRCTRL_TAG = "PwrCtrl"
XML_REGISTER_TAG = "Register"
XML_MASK_TAG = "Mask"
XML_MUXES_TAG = "Muxes"
XML_MUX_TAG = "Mux"
XML_POLICY_TAG = "Policy"
XML_ACTIVE_TAG = "Active"
XML_DEFAULT_FAILURE_ACTION_TAG = "DefaultFailureAction"
XML_SPIFREQ_TAG = "SPIFreq"
XML_IS_PA_ROT_TAG = "IsPARoT"


def xml_extract_attrib (root, attrib_name, string, required=True):
    attrib = root.attrib.get(attrib_name)
    if not attrib:
        if required:
            print ("Missing {0} attribute in PCD".format (attrib_name))
        return None

    if string:
        attrib.encode("utf8")
    
    return attrib.strip ()

def xml_find_single_tag (root, tag_name, required=True):
    tag = root.findall (tag_name)
    if not tag:
        if required:
            print ("Missing {0} tag in PCD".format (tag_name))
        return None
    elif len (tag) > 1:
        print ("Too many {0} tags in PCD".format (tag_name))
        return None
    
    return tag[0]

def xml_extract_single_value (root, requests):
    result = {}

    for name, tag_name in requests.items ():
        tag = root.findall (tag_name)
        if not tag:
            print ("Missing {0} tag in PCD".format (tag_name))
            return None
        elif len (tag) > 1:
            print ("Too many {0} tags in PCD".format (tag_name))
            return None
        
        result.update ({name:tag[0].text.strip ()})
        
    return result

def process_pfm(root):
    xml = {}

    def process_region(root, version_id):
        region = {}

        addr = root.findall(XML_START_ADDR_TAG)

        if not addr or len(addr) > 1:
            print("Invalid number of StartAddr tags in Firmware: {0}".format(version_id))
            return None

        region["start"] = addr[0].text.strip()

        addr = root.findall(XML_END_ADDR_TAG)

        if not addr or len(addr) > 1:
            print("Invalid number of EndAddr tags in Firmware: {0}".format(version_id))
            return None

        region["end"] = addr[0].text.strip()

        return region

    version_id = root.attrib.get(XML_VERSION_ATTRIB)

    if version_id is None:
        print("No Firmware version ID provided")
        return None

    platform_id = root.attrib.get(XML_PLATFORM_ATTRIB)

    if platform_id is None:
        print("No Platform ID provided")
        return None

    xml["version_id"] = version_id.strip().encode("utf8")
    xml["platform_id"] = platform_id.strip().encode("utf8")

    version = root.findall(XML_VERSION_ADDR_TAG)

    if not version or len(version) > 1:
        print("Invalid number of VersionAddr tags in Firmware: {0}".format(xml["version_id"]))
        return None

    xml["version_addr"] = version[0].text.strip()

    unused_byte = root.findall(XML_UNUSED_BYTE_TAG)

    if len(unused_byte) > 1:
        print("Invalid number of UnusedByte tags in Firmware: {0}".format(xml["version_id"]))
        return None

    if unused_byte:
        xml["unused_byte"] = unused_byte[0].text.strip()
    else:
        xml["unused_byte"] = "0xff"

    xml["rw_regions"] = []

    for rw in root.findall(XML_RW_TAG):
        for region in rw.findall(XML_REGION_TAG):
            processed_region = process_region(region, xml["version_id"])

            if processed_region is None:
                return None

            xml["rw_regions"].append(processed_region)

    xml["signed_imgs"] = []
    for img in root.findall(XML_IMG_TAG):
        image = {}
        image["regions"] = []

        pbkey = img.findall(XML_PB_KEY_TAG)

        if not pbkey or len (pbkey) > 1:
            print("Invalid number of PublicKey tags in SignedImage, Firmware {0}".format (xml["version_id"]))
            return None

        image["pbkey"] = pbkey[0].text.strip()

        sig = img.findall(XML_SIG_TAG)

        if not sig or len (sig) > 1:
            print("Invalid number of Signature tags in SignedImage, Firmware {0}".format (xml["version_id"]))
            return None

        image["signature"] = binascii.a2b_hex(re.sub("\s", "", sig[0].text.strip()))

        for region in img.findall(XML_REGION_TAG):
            processed_region = process_region(region, xml["version_id"])

            if processed_region is None:
                return None

            image["regions"].append(processed_region)

        if not image["regions"]:
            print("No regions found for SignedImage, Firmware: {0}".format(xml["version_id"]))
            return None

        prop = img.findall(XML_VALIDATE_TAG)

        if not prop or len (prop) > 1:
            print("Invalid number of ValidateOnBoot tags in SignedImage, Firmware {0}".format (xml["version_id"]))
            return None

        image["validate"] = prop[0].text.strip()

        xml["signed_imgs"].append(image)

    if not xml["signed_imgs"]:
        print("No signed images found for Firmware: {0}".format(xml["version_id"]))
        return None

    return xml

def process_cfm(root):
    xml = {}
    xml["fw_list"] = []

    device_id = root.attrib.get(XML_ID_ATTRIB)

    if device_id is None:
        print("No Device ID provided")
        return None

    xml["device_id"] = device_id.strip()

    for fw in root.findall(XML_FW_TAG):
        firmware = {}
        firmware["signed_imgs"] = []

        version = fw.attrib.get(XML_VERSION_ATTRIB)

        if version is None:
            print("No Firmware version provided for Device: {0}".format(xml["device_id"]))
            return None

        firmware["version"] = version.strip()

        for img in fw.findall(XML_IMG_TAG):
            image = {}

            digest = img.findall(XML_DIGEST_TAG)

            if not digest or len(digest) > 1:
                print("Invalid number of Digest tags in Device: {0}, Firmware: {1}".format(xml["device_id"], firmware["version"]))
                return None

            image["digest"] = binascii.a2b_hex(re.sub("\s", "", digest[0].text.strip()))

            action = img.findall(XML_FAILURE_ACTION_TAG)

            if not action or len(action) > 1:
                print("Invalid number of FailureAction tags in Device: {0}, Firmware: {1}".format(xml["device_id"], firmware["version"]))
                return None

            image["failure_action"] = action[0].text.strip()

            firmware["signed_imgs"].append(image)

        if not firmware["signed_imgs"]:
            print("No signed images found for Device: {0}, Firmware: {1}".format(xml["device_id"], firmware["version"]))
            return None

        xml["fw_list"].append(firmware)

    if not xml["fw_list"]:
        print("No firmware found for Device: {0}".format(xml["device_id"]))
        return None

    return xml

def process_pcd (root):
    xml = {}

    result = xml_extract_attrib (root, XML_PLATFORM_ATTRIB, True)
    if result is None:
        return None

    xml.update ({"platform_id":result})

    result = xml_extract_single_value (root, {"version": XML_VERSION_TAG})
    if result is None:
        return None

    xml.update (result)

    rot = xml_find_single_tag (root, XML_ROT_TAG)
    if rot is None:
        return None

    xml["rot"] = {}
    
    ports = xml_find_single_tag (rot, XML_PORTS_TAG, False)
    if ports is not None:
        xml["rot"]["ports"] = {}
        
        for port in ports.findall (XML_PORT_TAG):
            port_id = xml_extract_attrib (port, XML_ID_ATTRIB, False)
            if port_id is None:
                return None
                
            result = xml_extract_single_value (port, {"spifreq": XML_SPIFREQ_TAG})
            if result is None:
                return None
            
            xml["rot"]["ports"].update({port_id:result})

    interface = xml_find_single_tag (rot, XML_INTERFACE_TAG)
    if interface is None:
        return None

    result = xml_extract_single_value (rot, {"is_pa_rot": XML_IS_PA_ROT_TAG})
    if result is None:
        return None

    xml["rot"].update (result)

    result = xml_extract_single_value (interface, {"address": XML_ADDRESS_TAG, 
        "bmc_address": XML_BMC_ADDRESS_TAG})
    if result is None:
        return None

    xml["rot"]["interface"] = result

    cpld = xml_find_single_tag (root, XML_CPLD_TAG)
    if cpld is None:
        return None

    result = xml_extract_single_value (cpld, {"address": XML_ADDRESS_TAG, 
        "channel": XML_CHANNEL_TAG})
    if result is None:
        return None

    xml["cpld"] = result

    components = xml_find_single_tag (root, XML_COMPONENTS_TAG, False)
    if components is not None:
        xml["components"] = []
        
        for component in components.findall(XML_COMPONENT_TAG):
            result = xml_extract_single_value (component, {"devicetype": XML_DEVICETYPE_TAG, 
                "bus": XML_BUS_TAG, "address": XML_ADDRESS_TAG, "i2cmode": XML_I2CMODE_TAG,
                "eid": XML_EID_TAG})
            if result is None:
                return None

            pwrctl = xml_find_single_tag (component, XML_PWRCTRL_TAG)
            if pwrctl is None:
                return None

            pwrctl_result = xml_extract_single_value (pwrctl, {"register": XML_REGISTER_TAG, 
                "mask": XML_MASK_TAG})
            if pwrctl_result is None:
                return None

            result.update ({"powerctrl": pwrctl_result})

            muxes = xml_find_single_tag (component, XML_MUXES_TAG, False)
            if muxes is not None:
                muxes_list = {}
                
                for mux in muxes.findall (XML_MUX_TAG):
                    mux_result = xml_extract_single_value (mux, {"address": XML_ADDRESS_TAG, 
                        "channel": XML_CHANNEL_TAG})
                    if mux_result is None:
                        return None
                    
                    mux_level = xml_extract_attrib (mux, XML_LEVEL_ATTRIB, False)
                    if mux_level is None:
                        return None

                    muxes_list.update ({mux_level: mux_result})

                result.update ({"muxes": muxes_list})
            
            xml["components"].append(result)

    policy = xml_find_single_tag (root, XML_POLICY_TAG)
    if policy is None:
        return None

    result = xml_extract_single_value (policy, {"active": XML_ACTIVE_TAG, 
        "defaultfailureaction": XML_DEFAULT_FAILURE_ACTION_TAG})
    if result is None:
        return None

    xml["policy"] = result

    return xml

def load_and_process_xml (xml_file, xml_type):
    try:
        root = et.parse(xml_file).getroot()

        if xml_type is manifest_types.PFM:
            return process_pfm(root)
        elif xml_type is manifest_types.CFM:
            return process_cfm(root)
        elif xml_type is manifest_types.PCD:
            return process_pcd(root)
        else:
            print("Unknown XML type: {0}".format(xml_type))
            return None

    except Exception:
        print ("load_and_process_xml Exception")
        traceback.print_exc ()

        return None
