object ContentsForm: TContentsForm
  Left = 9
  Top = 9
  Width = 663
  Height = 634
  Caption = #1057#1086#1076#1077#1088#1078#1072#1085#1080#1077
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  Menu = MainMenu1
  OldCreateOrder = False
  OnResize = FormResize
  PixelsPerInch = 96
  TextHeight = 13
  object ContentsDrawGrid: TDrawGrid
    Left = 0
    Top = 0
    Width = 655
    Height = 588
    Align = alClient
    ColCount = 2
    DefaultColWidth = 300
    DefaultRowHeight = 20
    FixedCols = 0
    Options = [goFixedVertLine, goFixedHorzLine, goVertLine, goHorzLine, goRangeSelect, goRowSizing, goColSizing, goThumbTracking]
    TabOrder = 0
    OnClick = ContentsDrawGridClick
    OnDrawCell = ContentsDrawGridDrawCell
    OnKeyDown = ContentsDrawGridKeyDown
    OnSelectCell = ContentsDrawGridSelectCell
    ColWidths = (
      360
      48)
  end
  object CurCellMemo: TMemo
    Left = 192
    Top = 136
    Width = 185
    Height = 89
    BevelKind = bkFlat
    BorderStyle = bsNone
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    TabOrder = 1
    Visible = False
    OnExit = CurCellMemoExit
    OnKeyDown = CurCellMemoKeyDown
  end
  object MainMenu1: TMainMenu
    Left = 416
    Top = 208
    object N1: TMenuItem
      Caption = #1060#1072#1081#1083
      object doc1: TMenuItem
        Caption = #1042#1099#1074#1086#1076' '#1089#1086#1076#1077#1088#1078#1072#1085#1080#1103' '#1074' '#1092#1072#1081#1083' .doc ...'
        OnClick = doc1Click
      end
      object N3: TMenuItem
        Caption = #1042#1099#1093#1086#1076
        OnClick = N3Click
      end
    end
    object N2: TMenuItem
      Caption = #1056#1077#1076#1072#1082#1090#1080#1088#1086#1074#1072#1090#1100
      object N4: TMenuItem
        Caption = #1053#1072#1081#1090#1080'-'#1091#1076#1072#1083#1080#1090#1100' ...'
        OnClick = N4Click
      end
      object N5: TMenuItem
        Caption = #1059#1082#1072#1079#1072#1090#1100' '#1085#1086#1084#1077#1088' '#1087#1077#1088#1074#1086#1081' '#1089#1090#1088#1072#1085#1080#1094#1099'...'
        OnClick = N5Click
      end
    end
  end
end
