import CoreGraphics
import Foundation
// usage: winid [owner]  -> window number of the owner's main (layer 0) window
// On-screen windows first; when the screen is locked or asleep nothing is on screen, so fall
// back to the owner's largest layer-0 window (screencapture -l can still capture it).
let owner = CommandLine.arguments.count > 1 ? CommandLine.arguments[1] : "FreeWRL"
func windows(_ opt: CGWindowListOption) -> [[String: Any]] {
    let list = CGWindowListCopyWindowInfo(opt, kCGNullWindowID) as? [[String: Any]] ?? []
    return list.filter { ($0[kCGWindowOwnerName as String] as? String) == owner && ($0[kCGWindowLayer as String] as? Int) == 0 }
}
func area(_ w: [String: Any]) -> Double {
    let b = w[kCGWindowBounds as String] as? [String: Double] ?? [:]
    return (b["Width"] ?? 0) * (b["Height"] ?? 0)
}
if let w = windows([.optionOnScreenOnly]).first {
    print(w[kCGWindowNumber as String]!)
} else if let w = windows([.optionAll]).filter({ area($0) > 0 }).max(by: { area($0) < area($1) }) {
    print(w[kCGWindowNumber as String]!)
}
