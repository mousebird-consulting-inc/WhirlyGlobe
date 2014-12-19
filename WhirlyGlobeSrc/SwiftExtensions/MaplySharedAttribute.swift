//
//  MaplySharedAttribute.swift
//
// The idea is to use enums instead of string to name the keys, and use an array of keys instead 
// of a dictionary of strings, so that it can be type checked and auto-completed in XCode
// Enums in swift are polymorphic but typechecked, so that it's impossible to add, say, a string
// as a value for kMaplyFont
//
// Partial implementation only, need to extend it to support all possible keys
//
// Sample usage:
//     let labelStyle = [
//        kMaplyFont: UIFont.boldSystemFontOfSize(48.0),
//        kMaplyTextOutlineColor: UIColor.blackColor(),
//        kMaplyTextOutlineSize: 2.0,
//        kMaplyTextColor: UIColor(white: 1, alpha: 0.7)
//    ]

import UIKit

enum MaplySharedAttribute {
    
    case Filled(Bool)
    case Color(UIColor)
    case Selectable(Bool)
    case VecWidth(Float)
    case DrawPriority(Int)
    case TextColor(UIColor)
    case BackgroundColor(UIColor)
    case Font(UIFont)
    case LabelHeight(Float)
    case LabelWidth(Float)
    case TextOutlineColor(UIColor)
    case TextOutlineSize(Float)
    
    func addAttributeToDictionary(inout dictionary: [NSObject: AnyObject]) {
        switch self {
        case .Filled(let value): dictionary[kMaplyFilled] = value
        case .Color(let value): dictionary[kMaplyColor] = value
        case .Selectable(let value): dictionary[kMaplySelectable] = value
        case .VecWidth(let value): dictionary[kMaplyVecWidth] = value
        case .DrawPriority(let value): dictionary[kMaplyDrawPriority] = value
        case .TextColor(let value): dictionary[kMaplyTextColor] = value
        case .BackgroundColor(let value): dictionary[kMaplyBackgroundColor] = value
        case .Font(let value): dictionary[kMaplyFont] = value
        case .LabelHeight(let value): dictionary[kMaplyLabelHeight] = value
        case .LabelWidth(let value): dictionary[kMaplyLabelWidth] = value
        case .TextOutlineColor(let value): dictionary[kMaplyTextOutlineColor] = value
        case .TextOutlineSize(let value): dictionary[kMaplyTextOutlineSize] = value
        default: println("Unknown MaplySharedAttribute value: \(self)")
        }
    }
    
    func stringKey() -> String {
        switch self {
        case .Filled: return kMaplyFilled
        case .Color: return kMaplyColor
        case .Selectable: return kMaplySelectable
        case .VecWidth: return kMaplyVecWidth
        case .DrawPriority: return kMaplyDrawPriority
        case .TextColor: return kMaplyTextColor
        case .BackgroundColor: return kMaplyBackgroundColor
        case .Font: return kMaplyFont
        case .LabelHeight: return kMaplyLabelHeight
        case .LabelWidth: return kMaplyLabelWidth
        case .TextOutlineColor: return kMaplyTextOutlineColor
        case .TextOutlineSize: return kMaplyTextOutlineSize
        default: return ""
        }
    }
    

    static func dictionaryFromArray(array: [MaplySharedAttribute]) -> [NSObject: AnyObject] {
        var result = [NSObject: AnyObject]()
        for obj in array {
            obj.addAttributeToDictionary(&result)
        }
        return result
    }
    
}

