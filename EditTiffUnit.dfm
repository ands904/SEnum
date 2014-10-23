object EditTiffForm: TEditTiffForm
  Left = 19
  Top = 52
  Width = 583
  Height = 621
  Caption = #1056#1077#1076#1072#1082#1090#1080#1088#1086#1074#1072#1085#1080#1077' '#1092#1072#1081#1083#1072' *.tiff'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  KeyPreview = True
  OldCreateOrder = False
  OnCloseQuery = FormCloseQuery
  OnKeyDown = FormKeyDown
  PixelsPerInch = 96
  TextHeight = 13
  object Fog1: TFog
    Left = 0
    Top = 33
    Width = 559
    Height = 561
    Align = alClient
    TabOrder = 0
    OnPaint = Fog1Paint
    UseCompatibleDC = False
    OnMouseWheelUp = Fog1MouseWheelUp
    OnMouseWheelDown = Fog1MouseWheelDown
    object SaveFilePanel: TPanel
      Left = 56
      Top = 152
      Width = 321
      Height = 105
      TabOrder = 0
      Visible = False
    end
  end
  object Panel1: TPanel
    Left = 0
    Top = 0
    Width = 575
    Height = 33
    Align = alTop
    TabOrder = 1
    object RotateSpeedButton: TSpeedButton
      Left = 6
      Top = 5
      Width = 78
      Height = 22
      Hint = #1055#1077#1088#1077#1074#1077#1088#1085#1091#1090#1100' '#1090#1077#1082#1091#1097#1091#1102' '#1089#1090#1088#1072#1085#1080#1094#1091
      Caption = #1055#1077#1088#1077#1074#1077#1088#1085#1091#1090#1100
      Flat = True
      ParentShowHint = False
      ShowHint = True
      OnClick = RotateSpeedButtonClick
    end
    object EditSpeedButton: TSpeedButton
      Left = 84
      Top = 5
      Width = 90
      Height = 22
      Hint = #1056#1077#1076#1072#1082#1090#1080#1088#1086#1074#1072#1090#1100' '#1092#1072#1081#1083' .tiff ('#1079#1072#1084#1077#1085#1080#1090#1100', '#1091#1076#1072#1083#1080#1090#1100', '#1074#1089#1090#1072#1074#1080#1090#1100')'
      Caption = #1056#1077#1076#1072#1082#1090#1080#1088#1086#1074#1072#1090#1100
      Flat = True
      ParentShowHint = False
      ShowHint = True
      OnClick = EditSpeedButtonClick
    end
    object ConfigSpeedButton: TSpeedButton
      Left = 174
      Top = 5
      Width = 78
      Height = 22
      Hint = #1047#1072#1076#1072#1090#1100' '#1087#1072#1088#1072#1084#1077#1090#1088#1099' '#1088#1077#1076#1072#1082#1090#1080#1088#1086#1074#1072#1085#1080#1103' '#1092#1072#1081#1083#1072' .tiff'
      Caption = #1055#1072#1088#1072#1084#1077#1090#1088#1099
      Flat = True
      ParentShowHint = False
      ShowHint = True
      OnClick = ConfigSpeedButtonClick
    end
    object PageComboBox: TComboBox
      Left = 439
      Top = 5
      Width = 78
      Height = 21
      ItemHeight = 13
      TabOrder = 0
      Text = '0 / 0'
      OnSelect = PageComboBoxSelect
    end
    object IncBitBtn: TBitBtn
      Left = 520
      Top = 5
      Width = 25
      Height = 21
      Hint = #1057#1083#1077#1076#1091#1102#1097#1072#1103' '#1089#1090#1088#1072#1085#1080#1094#1072
      Caption = '>'
      ParentShowHint = False
      ShowHint = True
      TabOrder = 1
      OnClick = IncBitBtnClick
    end
    object DecBitBtn: TBitBtn
      Left = 411
      Top = 5
      Width = 25
      Height = 21
      Hint = #1055#1088#1077#1076#1099#1076#1091#1097#1072#1103' '#1089#1090#1088#1072#1085#1080#1094#1072
      Caption = '<'
      ParentShowHint = False
      ShowHint = True
      TabOrder = 2
      OnClick = DecBitBtnClick
    end
  end
  object ScrollBar1: TScrollBar
    Left = 559
    Top = 33
    Width = 16
    Height = 561
    Align = alRight
    Kind = sbVertical
    PageSize = 0
    Position = 15
    TabOrder = 2
    OnChange = ScrollBar1Change
  end
  object EditPopupMenu: TPopupMenu
    Left = 88
    Top = 56
    object ReplaceMenuItem: TMenuItem
      Caption = #1047#1072#1084#1077#1085#1080#1090#1100' '#1091#1082#1072#1079#1099#1074#1072#1077#1084#1099#1077' '#1089#1090#1088#1072#1085#1080#1094#1099
      Hint = #1047#1072#1084#1077#1085#1080#1090#1100' '#1091#1082#1072#1079#1072#1085#1085#1099#1081' '#1076#1080#1072#1087#1072#1079#1086#1085' '#1089#1090#1088#1072#1085#1080#1094' '#1076#1088#1091#1075#1080#1084' '#1092#1072#1081#1083#1086#1084
      OnClick = ReplaceMenuItemClick
    end
    object InsertMenuItem: TMenuItem
      Caption = #1042#1089#1090#1072#1074#1080#1090#1100' '#1087#1077#1088#1077#1076' '#1090#1077#1082#1091#1097#1077#1081' '#1089#1090#1088#1072#1085#1080#1094#1077#1081
      OnClick = InsertMenuItemClick
    end
    object AppendMenuItem: TMenuItem
      Caption = #1044#1086#1073#1072#1074#1080#1090#1100' '#1074' '#1082#1086#1085#1077#1094
      OnClick = AppendMenuItemClick
    end
    object DeleteMenuItem: TMenuItem
      Caption = #1059#1076#1072#1083#1080#1090#1100' '#1091#1082#1072#1079#1099#1074#1072#1077#1084#1099#1077' '#1089#1090#1088#1072#1085#1080#1094#1099
      OnClick = DeleteMenuItemClick
    end
  end
  object OpenTiffDialog: TOpenDialog
    DefaultExt = 'tif'
    Filter = #1060#1072#1081#1083#1099' *.tif|*.tif'
    Left = 192
    Top = 80
  end
  object SaveAsTiffDialog: TSaveDialog
    DefaultExt = 'tif'
    Filter = #1060#1072#1081#1083#1099' *.tif|*.tif'
    Options = [ofHideReadOnly, ofPathMustExist, ofEnableSizing]
    Left = 208
    Top = 136
  end
end
