import Foundation

// The objects in this file are used by generated code and should not need to be invoked manually.

public enum JSON {
    case Array([JSON])
    case Dictionary([String: JSON])
    case Str(String)
    case Number(NSNumber)
    case Null
}

func objectToJSON(json : AnyObject) -> JSON {
    
    switch json {
    case _ as NSNull:
        return .Null
    case let num as NSNumber:
        return .Number(num)
    case let str as String:
        return .Str(str)
    case let dict as [String : AnyObject]:
        var ret = [String : JSON]()
        for (k, v) in dict {
            ret[k] = objectToJSON(v)
        }
        return .Dictionary(ret)
    case let array as [AnyObject]:
        return .Array(array.map(objectToJSON))
    default:
        fatalError("Unknown type trying to parse JSON.")
    }
}

func prepareJSONForSerialization(json: JSON) -> AnyObject {
    switch json {
    case .Array(let array):
        return array.map(prepareJSONForSerialization)
    case .Dictionary(let dict):
        var ret = [String : AnyObject]()
        for (k, v) in dict {
            // kind of a hack...
            switch v {
            case .Null:
                continue
            default:
                ret[k] = prepareJSONForSerialization(v)
            }
        }
        return ret
    case .Number(let n):
        return n
    case .Str(let s):
        return s
    case .Null:
        return NSNull()
    }
}

func dumpJSON(json: JSON) -> NSData? {
    switch json {
    case .Null:
        return "null".dataUsingEncoding(NSUTF8StringEncoding, allowLossyConversion: false)
    default:
        let obj : AnyObject = prepareJSONForSerialization(json)
        if NSJSONSerialization.isValidJSONObject(obj) {
            return try! NSJSONSerialization.dataWithJSONObject(obj, options: NSJSONWritingOptions())
        } else {
            fatalError("Invalid JSON toplevel type")
        }
    }
}

func parseJSON(data: NSData) -> JSON {
    let obj: AnyObject = try! NSJSONSerialization.JSONObjectWithData(data, options: NSJSONReadingOptions.AllowFragments)
    return objectToJSON(obj)
    
}


public protocol JSONSerializer {
    typealias ValueType
    func serialize(_: ValueType) -> JSON
    func deserialize(_: JSON) -> ValueType
}

public class VoidSerializer : JSONSerializer {
    public func serialize(value: Void) -> JSON {
        return .Null
    }
    
    public func deserialize(json: JSON) -> Void {
        switch json {
        case .Null:
            return
        default:
            fatalError("Type error deserializing")
        }
        
    }
}


public class ArraySerializer<T : JSONSerializer> : JSONSerializer {
    
    var elementSerializer : T
    
    init(_ elementSerializer: T) {
        self.elementSerializer = elementSerializer
    }
    
    public func serialize(arr : Array<T.ValueType>) -> JSON {
        return .Array(arr.map { self.elementSerializer.serialize($0) } )
    }
    
    public func deserialize(json : JSON) -> Array<T.ValueType> {
        switch json {
        case .Array(let arr):
            return arr.map { self.elementSerializer.deserialize($0) }
        default:
            fatalError("Type error deserializing")
        }
    }
}

public class StringSerializer : JSONSerializer {
    public func serialize(value : String) -> JSON {
        return .Str(value)
    }
    
    public func deserialize(json: JSON) -> String {
        switch (json) {
        case .Str(let s):
            return s
        default:
            fatalError("Type error deserializing")
        }
    }
}

public class NSDateSerializer : JSONSerializer {
    
    var dateFormatter : NSDateFormatter
    
    private func convertFormat(format: String) -> String? {
        func symbolForToken(token: String) -> String {
            switch token {
            case "%a": // Weekday as locale’s abbreviated name.
                return "EEE"
            case "%A": // Weekday as locale’s full name.
                return "EEEE"
            case "%w": // Weekday as a decimal number, where 0 is Sunday and 6 is Saturday. 0, 1, ..., 6
                return "ccccc"
            case "%d": // Day of the month as a zero-padded decimal number. 01, 02, ..., 31
                return "dd"
            case "%b": // Month as locale’s abbreviated name.
                return "MMM"
            case "%B": // Month as locale’s full name.
                return "MMMM"
            case "%m": // Month as a zero-padded decimal number. 01, 02, ..., 12
                return "MM"
            case "%y": // Year without century as a zero-padded decimal number. 00, 01, ..., 99
                return "yy"
            case "%Y": // Year with century as a decimal number. 1970, 1988, 2001, 2013
                return "yyyy"
            case "%H": // Hour (24-hour clock) as a zero-padded decimal number. 00, 01, ..., 23
                return "HH"
            case "%I": // Hour (12-hour clock) as a zero-padded decimal number. 01, 02, ..., 12
                return "hh"
            case "%p": // Locale’s equivalent of either AM or PM.
                return "a"
            case "%M": // Minute as a zero-padded decimal number. 00, 01, ..., 59
                return "mm"
            case "%S": // Second as a zero-padded decimal number. 00, 01, ..., 59
                return "ss"
            case "%f": // Microsecond as a decimal number, zero-padded on the left. 000000, 000001, ..., 999999
                return "SSSSSS"
            case "%z": // UTC offset in the form +HHMM or -HHMM (empty string if the the object is naive). (empty), +0000, -0400, +1030
                return "Z"
            case "%Z": // Time zone name (empty string if the object is naive). (empty), UTC, EST, CST
                return "z"
            case "%j": // Day of the year as a zero-padded decimal number. 001, 002, ..., 366
                return "DDD"
            case "%U": // Week number of the year (Sunday as the first day of the week) as a zero padded decimal number. All days in a new year preceding the first Sunday are considered to be in week 0. 00, 01, ..., 53 (6)
                return "ww"
            case "%W": // Week number of the year (Monday as the first day of the week) as a decimal number. All days in a new year preceding the first Monday are considered to be in week 0. 00, 01, ..., 53 (6)
                return "ww" // one of these can't be right
            case "%c": // Locale’s appropriate date and time representation.
                return "" // unsupported
            case "%x": // Locale’s appropriate date representation.
                return "" // unsupported
            case "%X": // Locale’s appropriate time representation.
                return "" // unsupported
            case "%%": // A literal '%' character.
                return "%"
            default:
                return ""
            }
        }
        var newFormat = ""
        var inQuotedText = false
        var i = format.startIndex
        while i < format.endIndex {
            if format[i] == "%" {
                if i.successor() >= format.endIndex {
                    return nil
                }
                i = i.successor()
                let token = "%\(format[i])"
                if inQuotedText {
                    newFormat += "'"
                    inQuotedText = false
                }
                newFormat += symbolForToken(token)
            } else {
                if "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ".characters.contains(format[i]) {
                    if !inQuotedText {
                        newFormat += "'"
                        inQuotedText = true
                    }
                } else if format[i] == "'" {
                    newFormat += "'"
                }
                newFormat += String(format[i])
            }
            i = i.successor()
        }
        if inQuotedText {
            newFormat += "'"
        }
        return newFormat
    }
    
    
    init(_ dateFormat: String) {
        self.dateFormatter = NSDateFormatter()
        dateFormatter.dateFormat = self.convertFormat(dateFormat)
    }
    public func serialize(value: NSDate) -> JSON {
        return .Str(self.dateFormatter.stringFromDate(value))
    }
    
    public func deserialize(json: JSON) -> NSDate {
        switch json {
        case .Str(let s):
            return self.dateFormatter.dateFromString(s)!
        default:
            fatalError("Type error deserializing")
        }
    }
}

public class BoolSerializer : JSONSerializer {
    public func serialize(value : Bool) -> JSON {
        return .Number(NSNumber(bool: value))
    }
    public func deserialize(json : JSON) -> Bool {
        switch json {
        case .Number(let b):
            return b.boolValue
        default:
            fatalError("Type error deserializing")
        }
    }
}

public class UInt64Serializer : JSONSerializer {
    public func serialize(value : UInt64) -> JSON {
        return .Number(NSNumber(unsignedLongLong: value))
    }
    
    public func deserialize(json : JSON) -> UInt64 {
        switch json {
        case .Number(let n):
            return n.unsignedLongLongValue
        default:
            fatalError("Type error deserializing")
        }
    }
}

public class Int64Serializer : JSONSerializer {
    public func serialize(value : Int64) -> JSON {
        return .Number(NSNumber(longLong: value))
    }
    
    public func deserialize(json : JSON) -> Int64 {
        switch json {
        case .Number(let n):
            return n.longLongValue
        default:
            fatalError("Type error deserializing")
        }
    }
}

public class Int32Serializer : JSONSerializer {
    public func serialize(value : Int32) -> JSON {
        return .Number(NSNumber(int: value))
    }
    
    public func deserialize(json : JSON) -> Int32 {
        switch json {
        case .Number(let n):
            return n.intValue
        default:
            fatalError("Type error deserializing")
        }
    }
}
public class UInt32Serializer : JSONSerializer {
    public func serialize(value : UInt32) -> JSON {
        return .Number(NSNumber(unsignedInt: value))
    }
    
    public func deserialize(json : JSON) -> UInt32 {
        switch json {
        case .Number(let n):
            return n.unsignedIntValue
        default:
            fatalError("Type error deserializing")
        }
    }
}

public class NSDataSerializer : JSONSerializer {
    public func serialize(value : NSData) -> JSON {
        return .Str(value.base64EncodedStringWithOptions([]))
    }
    
    public func deserialize(json: JSON) -> NSData {
        switch(json) {
        case .Str(let s):
            return NSData(base64EncodedString: s, options: [])!
        default:
            fatalError("Type error deserializing")
        }
    }
}

public class DoubleSerializer : JSONSerializer {
    public func serialize(value: Double) -> JSON {
        return .Number(NSNumber(double: value))
    }
    
    public func deserialize(json: JSON) -> Double {
        switch json {
        case .Number(let n):
            return n.doubleValue
        default:
            fatalError("Type error deserializing")
        }
    }
}


public class NullableSerializer<T : JSONSerializer> : JSONSerializer {
    
    var internalSerializer : T
    
    init(_ serializer : T) {
        self.internalSerializer = serializer
    }
    
    public func serialize(value : Optional<T.ValueType>) -> JSON {
        if let v = value {
            return internalSerializer.serialize(v)
        } else {
            return .Null
        }
    }
    
    public func deserialize(json: JSON) -> Optional<T.ValueType> {
        switch json {
        case .Null:
            return nil
        default:
            return internalSerializer.deserialize(json)
        }
    }
}

struct Serialization {
    static var _StringSerializer = StringSerializer()
    static var _BoolSerializer = BoolSerializer()
    static var _UInt64Serializer = UInt64Serializer()
    static var _UInt32Serializer = UInt32Serializer()
    static var _Int64Serializer = Int64Serializer()
    static var _Int32Serializer = Int32Serializer()

    static var _VoidSerializer = VoidSerializer()
    static var _NSDataSerializer = NSDataSerializer()
    static var _DoubleSerializer = DoubleSerializer()

    static func getFields(json : JSON) -> [String : JSON] {
        switch json {
            case .Dictionary(let dict):
                return dict
            default:
                fatalError("Type error")
        }
    }

    static func getTag(d: [String : JSON]) -> String {
        return _StringSerializer.deserialize(d[".tag"]!)
    }

}

