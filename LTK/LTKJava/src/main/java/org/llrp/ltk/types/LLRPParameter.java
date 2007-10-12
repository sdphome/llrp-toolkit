/*
 * Copyright 2007 ETH Zurich
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 *
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and limitations under the License.
 */
package org.llrp.ltk.types;

import org.jdom.Content;
import org.jdom.Element;


/**
 * Representing an LLRPParameter uniquely identified by its type number.
 *
 * @author Basil Gasser - ETH Zurich
 */
public abstract class LLRPParameter {
    protected UnsignedShort bitLength = new UnsignedShort();

    /**
     * type num uniquely identifies a parameter.
     *
     * @return typeNum
     */
    public abstract SignedShort getTypeNum();

    /**
     * create objects from binary.
     *
     * @param list - created by calling encodeBinary()
     */
    public abstract void decodeBinary(LLRPBitList list);

    /**
     * create binary representation of this parameter.
     *
     * @return LLRPBitList - a list of bits
     */
    public abstract LLRPBitList encodeBinary();

    /**
     * create xml representation of this parameter.
     *
     * @param root element to add childs
     */
    public abstract Content encodeXML(String name);

    /**
     * create objects from xml.
     *
     * @param dom element
     */
    public abstract void decodeXML(Element element);

    /**
     * length in bytes.
     *
     * @return length of message in bytes (octet)
     */
    public Integer getByteLength() {
        if ((bitLength == null) || bitLength.equals(new UnsignedShort(0))) {
            bitLength = new UnsignedShort(encodeBinary().length());
        }

        return (bitLength.toShort() / 8);
    }
}